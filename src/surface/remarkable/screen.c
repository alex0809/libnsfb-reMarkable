#define _GNU_SOURCE // required for naming threads

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "screen.h"
#include "mxcfb.h"
#include "log.h"

#define UNUSED(x) ((x) = (x))

int fb_claim_region(fb_state_t *fb_state, nsfb_bbox_t *box) {
    UNUSED(box);
    if (pthread_mutex_lock(&fb_state->fb_mutex) != 0) {
        ERROR_LOG("fb_claim_region: Could not lock update mutex.");
        return -1;
    }
    return 0;
}

int fb_update_region(fb_state_t *fb_state, nsfb_bbox_t *box)
{
    UNUSED(box);
    fb_state->next_update_x0 = MIN(fb_state->next_update_x0, box->x0);
    fb_state->next_update_x1 = MAX(fb_state->next_update_x1, box->x1);
    fb_state->next_update_y0 = MIN(fb_state->next_update_y0, box->y0);
    fb_state->next_update_y1 = MAX(fb_state->next_update_y1, box->y1);
    if (pthread_mutex_unlock(&fb_state->fb_mutex) != 0) {
        ERROR_LOG("fb_update_region: could not unlock update mutex.");
    }
    return 0;
}

int fb_initialize(fb_state_t *fb_state)
{
    int fb = open(FRAMEBUFFER_FILE, O_RDWR);
    if (fb == -1) {
        ERROR_LOG("fb_initialize: could not open framebuffer");
        return -1;
    }
    fb_state->fb = fb;
    DEBUG_LOG("fb_initialize: Framebuffer %s opened", FRAMEBUFFER_FILE);

    struct fb_fix_screeninfo f_screen_info;
    if (ioctl(fb_state->fb, FBIOGET_FSCREENINFO, &f_screen_info) != 0)
    {
        ERROR_LOG("fb_initialize: could not FBIOGET_FSCREENFINO");
        return -1;
    }
    struct fb_var_screeninfo v_screen_info;
    if (ioctl(fb_state->fb, FBIOGET_VSCREENINFO, &v_screen_info) != 0)
    {
        ERROR_LOG("fb_initialize: could not FBIOGET_VSCREENFINO");
        return -1;
    }
    fb_state->scrinfo.width = v_screen_info.xres;
    fb_state->scrinfo.height = v_screen_info.yres;
    fb_state->scrinfo.linelen = f_screen_info.line_length;
    fb_state->scrinfo.bpp = v_screen_info.bits_per_pixel;
    fb_state->fb_size = v_screen_info.yres_virtual * f_screen_info.line_length;
    DEBUG_LOG("fb_initialize: Screeninfo loaded");

    void *mmap_result = mmap(NULL, fb_state->fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (mmap_result == MAP_FAILED)
    {
        ERROR_LOG("fb_initialize: Framebuffer mmap failed. Exiting.");
        return -1;
    }
    fb_state->mapped_fb = mmap_result;

    DEBUG_LOG("fb_initialize: mmapped %d bytes of framebuffer", fb_state->fb_size);

    pthread_t thread;
    int thread_create_result = pthread_create(&thread, NULL, fb_async_redraw, fb_state);
    pthread_setname_np(thread, "screen");
    if (thread_create_result != 0) {
        ERROR_LOG("fb_initialize: could not initialize async update thread");
        return -1;
    }
    fb_state->redraw_active = true;
    fb_state->redraw_thread = thread;
    pthread_mutex_t fb_mutex = PTHREAD_MUTEX_INITIALIZER;
    fb_state->fb_mutex = fb_mutex;

    return 0;
}

void *fb_async_redraw(void *context) 
{
    struct timespec millisecond_sleep;
    millisecond_sleep.tv_nsec = 200000000;
    millisecond_sleep.tv_sec = 0;

    fb_state_t *state = (fb_state_t*)context;
    while (state->redraw_active) {
        if (pthread_mutex_lock(&state->fb_mutex) != 0) {
            ERROR_LOG("fb_async_redraw: Redrawing thread could not lock mutex.");
        }

        if (state->next_update_x1 != 0 && state->next_update_y1 != 0) {
            struct mxcfb_update_data update_data;
            struct mxcfb_rect update_rect;
            update_rect.left = state->next_update_x0;
            update_rect.width = (state->next_update_x1 - state->next_update_x0);
            update_rect.top = state->next_update_y0;
            update_rect.height = (state->next_update_y1 - state->next_update_y0);
    
            update_data.update_region = update_rect;
            update_data.waveform_mode = DEFAULT_WAVEFORM_MODE;
            update_data.update_mode = UPDATE_MODE_PARTIAL;
            update_data.update_marker = 0;
            update_data.dither_mode = DEFAULT_EPDC_FLAG;
            update_data.temp = DEFAULT_TEMP;
            update_data.flags = 0;
    
            if (ioctl(state->fb, MXCFB_SEND_UPDATE, &update_data) == -1) {
                ERROR_LOG("fb_async_redraw: error executing MXCFB_SEND_UPDATE!");
            }
            TRACE_LOG("fb_async_redraw: Sent MXCFB_SEND_UPDATE ioctl for region: left=%d, width=%d, top=%d, height=%d",
                update_rect.left, update_rect.width, update_rect.top, update_rect.height);

            state->next_update_x0 = 0;
            state->next_update_x1 = 0;
            state->next_update_y0 = 0;
            state->next_update_y1 = 0;
        }

        pthread_mutex_unlock(&state->fb_mutex);
        nanosleep(&millisecond_sleep, &millisecond_sleep);
    }
    return 0;
}

int fb_finalize(fb_state_t *fb_state) {
    if (close(fb_state->fb) < 0) {
        DEBUG_LOG("fb_finalize: could not close fb");
        return -1;
    }
    if (munmap(fb_state->mapped_fb, fb_state->fb_size) != 0) {
        DEBUG_LOG("fb_finalize: could not munmap");
        return -1;
    }
    fb_state->redraw_active = false;
    DEBUG_LOG("Waiting for redraw thread to exit");
    pthread_join(fb_state->redraw_thread, NULL);
    DEBUG_LOG("Redraw thread exited");
    return 0;
}

