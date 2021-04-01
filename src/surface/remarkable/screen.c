#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>

#include "screen.h"
#include "mxcfb.h"

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
        fprintf(stderr, "load_screen_info: could not FBIOGET_FSCREENFINO\n");
        return -1;
    }
    struct fb_var_screeninfo v_screen_info;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &v_screen_info) != 0)
    {
        close_fb();
        fprintf(stderr, "load_screen_info: could not FBIOGET_VSCREENFINO\n");
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
    return 0;
}

int open_fb()
{
    if (fb != -1) {
        return 0;
    }

    fb = open("/dev/fb0", O_RDWR);
    if (fb == -1) {
        fprintf(stderr, "Could not open framebuffer\n");
        return -1;
    }

    return 0;
}

int close_fb()
{
    if (fb == -1) {
        return 0;
    }
    return 0;
}
