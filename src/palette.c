/*
 * Copyright 2012 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/** \file
 * Palette (implementation).
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "palette.h"


/** Create an empty palette object. */
bool nsfb_palette_new(struct nsfb_palette_s **palette)
{
	*palette = malloc(sizeof(struct nsfb_palette_s));
	if (*palette == NULL) {
		return false;
	}

	(*palette)->type = NSFB_PALETTE_EMPTY;
	(*palette)->last = 0;

	return true;
}

/** Free a palette object. */
void nsfb_palette_free(struct nsfb_palette_s *palette)
{
	if (palette != NULL)
		free(palette);
}

/** Generate libnsfb 8bpp default palette. */
void nsfb_palette_generate_nsfb_8bpp(struct nsfb_palette_s *palette)
{
	int rloop, gloop, bloop;
	int loop = 0;
	uint8_t r, g, b;

	/* Build a linear 6-8-5 levels RGB colour cube palette.
	 * This accounts for 240 colours */
#define RLIM 6
#define GLIM 8
#define BLIM 5
	for (rloop = 0; rloop < RLIM; rloop++) {
		for (gloop = 0; gloop < GLIM; gloop++) {
			for (bloop = 0; bloop < BLIM; bloop++) {
				r = ((rloop * 255 * 2) + RLIM - 1) /
						(2 * (RLIM - 1));
				g = ((gloop * 255 * 2) + GLIM - 1) /
						(2 * (GLIM - 1));
				b = ((bloop * 255 * 2) + BLIM - 1) /
						(2 * (BLIM - 1));

				palette->data[loop] = r | g << 8 | b << 16;
				loop++;
			}
		}
	}
#undef RLIM
#undef GLIM
#undef BLIM

	/* Should have 240 colours set */
	assert(loop == 240);

	/* Fill index 240 to index 255 with grayscales */
	/* Note: already have full black and full white from RGB cube */
	for (; loop < 256; loop++) {
		int ngray = loop - 240 + 1;
		r = ngray * 15; /* 17*15 = 255 */

		g = b = r;

		palette->data[loop] = r | g << 8 | b << 16;
	}

	/* Set palette details */
	palette->type = NSFB_PALETTE_NSFB_8BPP;
	palette->last = 255;
}
