/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdio.h>

#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"

#include "nsfb.h"
#include "plot.h"
#include "frontend.h"


#define UNUSED(x) ((x) = (x))

static int linux_set_geometry(nsfb_t *nsfb, int width, int height, int bpp)
{
    if (nsfb->frontend_priv != NULL)
        return -1; /* if we are already initialised fail */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->bpp = bpp;

    /* select default sw plotters for bpp */
    select_plotters(nsfb);

    return 0;
}

static int linux_initialise(nsfb_t *nsfb)
{
    if (nsfb->frontend_priv != NULL)
        return -1;

    /* sanity checked depth. */
    if ((nsfb->bpp != 32) && (nsfb->bpp != 16) && (nsfb->bpp != 8))
        return -1;


    return 0;
}

static int linux_finalise(nsfb_t *nsfb)
{
    UNUSED(nsfb);
    return 0;
}

static bool linux_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

static int linux_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    if ((cursor != NULL) && 
        (cursor->plotted == true) && 
        (nsfb_plot_bbox_intersect(box, &cursor->loc))) {

        nsfb->plotter_fns->bitmap(nsfb, 
                                  &cursor->savloc,  
                                  cursor->sav, 
                                  cursor->sav_width, 
                                  cursor->sav_height, 
                                  cursor->sav_width, 
                                  false);
        cursor->plotted = false;
    }
    return 0;
}

static int linux_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
    nsfb_bbox_t sclip;

    if ((cursor != NULL) && (cursor->plotted == true)) {
        sclip = nsfb->clip;

        nsfb->plotter_fns->set_clip(nsfb, NULL);

        nsfb->plotter_fns->bitmap(nsfb, 
                                  &cursor->savloc,  
                                  cursor->sav, 
                                  cursor->sav_width, 
                                  cursor->sav_height, 
                                  cursor->sav_width, 
                                  false);

        nsfb_cursor_plot(nsfb, cursor);

        nsfb->clip = sclip;
    }
    return true;
}


static int linux_release(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    if ((cursor != NULL) && (cursor->plotted == false)) {
        nsfb_cursor_plot(nsfb, cursor);
    }

    return 0;
}

const nsfb_frontend_rtns_t linux_rtns = {
    .initialise = linux_initialise,
    .finalise = linux_finalise,
    .input = linux_input,
    .claim = linux_claim,
    .release = linux_release,
    .cursor = linux_cursor,
    .geometry = linux_set_geometry,
};

NSFB_FRONTEND_DEF(linux, NSFB_FRONTEND_LINUX, &linux_rtns)
