/*
 * Copyright 2012 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the *internal* interface for the cursor. 
 */

#ifndef PALETTE_H
#define PALETTE_H 1

#include <stdint.h>
#include <limits.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"

enum nsfb_palette_type_e {
	NSFB_PALETTE_EMPTY,     /**< empty palette object */
	NSFB_PALETTE_NSFB_8BPP, /**< libnsfb's own 8bpp palette */
	NSFB_PALETTE_OTHER      /**< any other palette  */
};

struct nsfb_palette_s {
	enum nsfb_palette_type_e type; /**< Palette type */
	uint8_t last; /**< Last used palette index */
	nsfb_colour_t data[256]; /**< Palette for index modes */
};

/** Create an empty palette object. */
bool nsfb_palette_new(struct nsfb_palette_s **palette);

/** Free a palette object. */
void nsfb_palette_free(struct nsfb_palette_s *palette);

/** Generate libnsfb 8bpp default palette. */
void nsfb_palette_generate_nsfb_8bpp(struct nsfb_palette_s *palette);


static inline uint8_t nsfb_palette_best_match(
		const struct nsfb_palette_s *palette,
		nsfb_colour_t c)
{
	uint8_t best_col = 0;

	nsfb_colour_t palent;
	int col;
	int dr, dg, db; /* delta red, green blue values */

	int cur_distance;
	int best_distance = INT_MAX;

	switch (palette->type) {
	case NSFB_PALETTE_NSFB_8BPP:
		/* Index into colour cube part */
		dr = ((( c        & 0xFF) * 5) + 128) / 256;
		dg = ((((c >>  8) & 0xFF) * 7) + 128) / 256;
		db = ((((c >> 16) & 0xFF) * 4) + 128) / 256;
		col = 40 * dr + 5 * dg + db;

		palent = palette->data[col];
		dr = ( c        & 0xFF) - ( palent        & 0xFF);
		dg = ((c >>  8) & 0xFF) - ((palent >> 8 ) & 0xFF);
		db = ((c >> 16) & 0xFF) - ((palent >> 16) & 0xFF);
		cur_distance = (dr * dr) + (dg * dg) + (db * db);

		best_col = col;
		best_distance = cur_distance;

		/* Index into grayscale part */
		col = (( c        & 0xFF) +
		       ((c >>  8) & 0xFF) +
		       ((c >> 16) & 0xFF) + (45 / 2)) / (15 * 3) - 1 + 240;
		palent = palette->data[col];

		dr = ( c        & 0xFF) - ( palent        & 0xFF);
		dg = ((c >>  8) & 0xFF) - ((palent >>  8) & 0xFF);
		db = ((c >> 16) & 0xFF) - ((palent >> 16) & 0xFF);
		cur_distance = (dr * dr) + (dg * dg) + (db * db);
		if (cur_distance < best_distance) {
			best_distance = cur_distance;
			best_col = col;
		}
		break;

	case NSFB_PALETTE_OTHER:
		/* Try all colours in palette */
		for (col = 0; col <= palette->last; col++) {
			palent = palette->data[col];

			dr = ( c        & 0xFF) - ( palent        & 0xFF);
			dg = ((c >>  8) & 0xFF) - ((palent >>  8) & 0xFF);
			db = ((c >> 16) & 0xFF) - ((palent >> 16) & 0xFF);
			cur_distance = (dr * dr) + (dg * dg) + (db * db);
			if (cur_distance < best_distance) {
				best_distance = cur_distance;
				best_col = col;
			}
		}
		break;

	default:
		break;
	}

        return best_col;
}

#endif /* PALETTE_H */
