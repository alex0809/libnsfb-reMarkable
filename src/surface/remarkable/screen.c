#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>

#include "screen.h"
#include "mxcfb.h"
#include "log.h"

int fb = -1;

int load_screen_info(struct screen_info *scrinfo)
{
    if (fb == -1) {
        open_fb();
    }

    struct fb_fix_screeninfo f_screen_info;
    if (ioctl(fb, FBIOGET_FSCREENINFO, &f_screen_info) != 0)
    {
        close_fb();
        ERROR_LOG("load_screen_info: could not FBIOGET_FSCREENFINO");
        return -1;
    }
    struct fb_var_screeninfo v_screen_info;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &v_screen_info) != 0)
    {
        close_fb();
        ERROR_LOG("load_screen_info: could not FBIOGET_VSCREENFINO");
        return -1;
    }

    scrinfo->width = v_screen_info.xres;
    scrinfo->height = v_screen_info.yres;
    scrinfo->linelen = f_screen_info.line_length;
    scrinfo->bpp = v_screen_info.bits_per_pixel;
    scrinfo->fbsize = v_screen_info.yres_virtual * f_screen_info.line_length;

    return 0;
}

int update_region(nsfb_bbox_t *box)
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

    ioctl(fb, MXCFB_SEND_UPDATE, &update_data);

    TRACE_LOG("Sent MXCFB_SEND_UPDATE ioctl for region: left=%d, width=%d, top=%d, height=%d",
            update_rect.left, update_rect.width, update_rect.top, update_rect.height);
    return 0;
}

int open_fb()
{
    if (fb != -1) {
        return 0;
    }

    fb = open(FRAMEBUFFER_FILE, O_RDWR);
    if (fb == -1) {
        ERROR_LOG("Could not open framebuffer");
        return -1;
    }

    DEBUG_LOG("Framebuffer %s opened", FRAMEBUFFER_FILE);

    return 0;
}

int close_fb()
{
    if (fb == -1) {
        return 0;
    }
    return 0;
}
