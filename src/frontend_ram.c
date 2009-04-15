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
#include "nsfb.h"
#include "frontend.h"

#define UNUSED(x) ((x) = (x))

static int ram_set_geometry(nsfb_t *nsfb, int width, int height, int bpp)
{
    if (nsfb->frontend_priv != NULL)
        return -1; /* if were already initialised fail */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->bpp = bpp;

    return 0;
}

static int ram_initialise(nsfb_t *nsfb)
{
    UNUSED(nsfb);
    return 0;
}

static int ram_finalise(nsfb_t *nsfb)
{
    UNUSED(nsfb);
    return 0;
}

static bool ram_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

const nsfb_frontend_rtns_t ram_rtns = {
    .initialise = ram_initialise,
    .finalise = ram_finalise,
    .input = ram_input,
    .geometry = ram_set_geometry,
};

NSFB_FRONTEND_DEF(ram, NSFB_FRONTEND_RAM, &ram_rtns)
