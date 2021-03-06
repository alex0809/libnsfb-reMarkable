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
#include <time.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#include "nsfb.h"
#include "surface.h"
#include "plot.h"
#include "screen.h"
#include "log.h"
#include "input.h"

#define UNUSED(x) ((x) = (x))

struct timespec millisecond_sleep;
input_state_t input_state;
fb_state_t fb_state;

static int rm_defaults(nsfb_t *nsfb)
{
	if (fb_initialize(&fb_state) != 0) {
		ERROR_LOG(
			"rm_defaults: could not successfully initialize framebuffer. Exiting.");
		exit(1);
	}

	nsfb->width = fb_state.scrinfo.width;
	nsfb->height = fb_state.scrinfo.height;
	nsfb->bpp = fb_state.scrinfo.bpp;
	nsfb->linelen = fb_state.scrinfo.linelen;
	nsfb->format = NSFB_FMT_RGB565;

	/* select default sw plotters for bpp */
	select_plotters(nsfb);

	DEBUG_LOG(
		"rm_defaults: Screen defaults set to: width=%d, height=%d, bpp=%d, linelen=%d",
		nsfb->width,
		nsfb->height,
		nsfb->bpp,
		nsfb->linelen);

	return 0;
}

static int rm_initialise(nsfb_t *nsfb)
{
	if (input_initialize(&input_state, nsfb) != 0) {
		ERROR_LOG(
			"rm_initialize: could not initialize input devices. Exiting");
		exit(1);
	}
	nsfb->ptr = fb_state.mapped_fb;

	return 0;
}

static int
rm_set_geometry(nsfb_t *nsfb, int width, int height, enum nsfb_format_e format)
{
	UNUSED(nsfb);
	UNUSED(width);
	UNUSED(height);
	UNUSED(format);
	DEBUG_LOG("rm_set_geometry: not implemented!");
	return 0;
}

static int rm_finalise(nsfb_t *nsfb)
{
	UNUSED(nsfb);
	fb_finalize(&fb_state);
	input_finalize(&input_state);

	return 0;
}

static bool rm_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
	UNUSED(nsfb);
	return input_get_next_event(&input_state, event, timeout);
}

static int rm_update(nsfb_t *nsfb, nsfb_bbox_t *box)
{
	UNUSED(nsfb);
	return fb_update_region(&fb_state, box);
}

static int rm_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
	UNUSED(nsfb);
	return fb_claim_region(&fb_state, box);
}

const nsfb_surface_rtns_t rm_rtns = {
	.defaults = rm_defaults,
	.initialise = rm_initialise,
	.finalise = rm_finalise,
	.input = rm_input,
	.update = rm_update,
	.claim = rm_claim,
	.geometry = rm_set_geometry,
};

NSFB_SURFACE_DEF(remarkable, NSFB_SURFACE_REMARKABLE, &rm_rtns)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
