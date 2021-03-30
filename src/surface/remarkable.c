/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#include "nsfb.h"
#include "surface.h"
#include "plot.h"

#include "mxcfb.h"

#define UNUSED(x) ((x) = (x))

static int rm_defaults(nsfb_t *nsfb)
{
    nsfb->width = 1404;
    nsfb->height = 1872;
    nsfb->bpp = 16;
    nsfb->linelen = 2816;
    nsfb->format = NSFB_FMT_RGB565;

    /* select default sw plotters for bpp */
    select_plotters(nsfb);

    return 0;
}


static int rm_initialise(nsfb_t *nsfb)
{
    int fb = open("/dev/fb0", O_RDWR);

    if (fb == -1) {
        fprintf(stderr, "Could not open framebuffer\n");
        exit(1);
    }

    struct fb_fix_screeninfo fixScreenInfo;
    if (ioctl(fb, FBIOGET_FSCREENINFO, &fixScreenInfo) != 0)
    {
        close(fb);
        fprintf(stderr, "FrameBuffer_initialize: could not FBIOGET_FSCREENFINO\n");
        return -1;
    }
    struct fb_var_screeninfo screenInfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &screenInfo) != 0)
    {
        close(fb);
        fprintf(stderr, "FrameBuffer_initialize: could not FBIOGET_VSCREENFINO\n");
        return -1;
    }

    // Memory-map the data buffer.
    int fbsize = screenInfo.yres_virtual * fixScreenInfo.line_length;
    u_int8_t* mmap_result = mmap(0, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (mmap_result == MAP_FAILED)
    {
        close(fb);
        fprintf(stderr, "mmap failed.\n");
        return -1;
    }

    nsfb->ptr = mmap_result;
    return 0;
}

static int
rm_set_geometry(nsfb_t *nsfb, int width, int height, enum nsfb_format_e format)
{
    fprintf(stderr, "rm_set_geometry not implemented!\n");
    return 0;
}


static int rm_finalise(nsfb_t *nsfb)
{
    free(nsfb->ptr);

    return 0;
}

static bool rm_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

const nsfb_surface_rtns_t rm_rtns = {
    .defaults = rm_defaults,
    .initialise = rm_initialise,
    .finalise = rm_finalise,
    .input = rm_input,
    .geometry = rm_set_geometry,
};

NSFB_SURFACE_DEF(remarkable, NSFB_SURFACE_REMARKABLE, &rm_rtns)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
