#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>

#include "screen.h"
#include "mxcfb.h"
#include "log.h"

int fb_update_region(fb_state_t *fb_state, nsfb_bbox_t *box)
{
    struct mxcfb_update_data update_data;
    struct mxcfb_rect update_rect;
    update_rect.left = box->x0;
    update_rect.width = (box->x1 - box->x0);
    update_rect.top = box->y0;
    update_rect.height = (box->y1 - box->y0);

    update_data.update_region = update_rect;
    update_data.waveform_mode = DEFAULT_WAVEFORM_MODE;
    update_data.update_mode = UPDATE_MODE_PARTIAL;
    update_data.update_marker = 0;
    update_data.dither_mode = DEFAULT_EPDC_FLAG;
    update_data.temp = DEFAULT_TEMP;
    update_data.flags = 0;

    if (ioctl(fb_state->fb, MXCFB_SEND_UPDATE, &update_data) == -1) {
        ERROR_LOG("fb_update_region: error executing MXCFB_SEND_UPDATE!");
        return -1;
    }

    TRACE_LOG("fb_update_region: Sent MXCFB_SEND_UPDATE ioctl for region: left=%d, width=%d, top=%d, height=%d",
            update_rect.left, update_rect.width, update_rect.top, update_rect.height);
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
    return 0;
}

