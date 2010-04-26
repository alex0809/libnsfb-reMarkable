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
#include "frontend.h"

#define UNUSED(x) ((x) = (x))

static int vnc_set_geometry(nsfb_t *nsfb, int width, int height, int bpp)
{
    if (nsfb->frontend_priv != NULL)
        return -1; /* if were already initialised fail */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->bpp = bpp;

    return 0;
}

static int vnc_initialise(nsfb_t *nsfb)
{
    UNUSED(nsfb);
    return 0;
}

static int vnc_finalise(nsfb_t *nsfb)
{
    UNUSED(nsfb);
    return 0;
}

static bool vnc_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

const nsfb_frontend_rtns_t vnc_rtns = {
    .initialise = vnc_initialise,
    .finalise = vnc_finalise,
    .input = vnc_input,
    .geometry = vnc_set_geometry,
};

NSFB_FRONTEND_DEF(vnc, NSFB_FRONTEND_VNC, &vnc_rtns)
