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
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#include "nsfb.h"
#include "surface.h"
#include "plot.h"
#include "screen.h"

#define UNUSED(x) ((x) = (x))

static int rm_defaults(nsfb_t *nsfb)
{
    struct screen_info scrinfo;
    if (load_screen_info(&scrinfo) != 0)
    {
        fprintf(stderr, "Could not successfully load screen info. Exiting.");
        exit(1);
    }

    nsfb->width = scrinfo.width;
    nsfb->height = scrinfo.height;
    nsfb->bpp = scrinfo.bpp;
    nsfb->linelen = scrinfo.linelen;
    nsfb->format = NSFB_FMT_RGB565;

    /* select default sw plotters for bpp */
    select_plotters(nsfb);

    fprintf(stderr, "Screen defaults set to: width=%d, height=%d, bpp=%d, linelen=%d\n",
            nsfb->width, nsfb->height, nsfb->bpp, nsfb->linelen);

    return 0;
}


static int rm_initialise(nsfb_t *nsfb)
{
    struct screen_info scrinfo;
    if (load_screen_info(&scrinfo) != 0)
    {
        fprintf(stderr, "Could not successfully load screen info. Exiting.\n");
        exit(1);
    }
    u_int8_t* mmap_result = mmap(NULL, scrinfo.fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (mmap_result == MAP_FAILED)
    {
        close_fb();
        fprintf(stderr, "Framebuffer mmap failed. Exiting.");
        exit(1);
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
    close_fb();

    return 0;
}

static bool rm_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

static int rm_update(nsfb_t *nsfb, nsfb_bbox_t *box) {
    return update_region(box);
}

const nsfb_surface_rtns_t rm_rtns = {
    .defaults = rm_defaults,
    .initialise = rm_initialise,
    .finalise = rm_finalise,
    .input = rm_input,
    .update = rm_update,
    .geometry = rm_set_geometry,
};

NSFB_SURFACE_DEF(remarkable, NSFB_SURFACE_REMARKABLE, &rm_rtns)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
