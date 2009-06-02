/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <endian.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "libnsfb.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "nsfb_plot.h"
#include "plotters.h"

static inline uint8_t *get_xy_loc(nsfb_t *nsfb, int x, int y)
{
        return (uint8_t *)(nsfb->ptr + (y * nsfb->linelen) + (x));
}


static inline nsfb_colour_t pixel_to_colour(nsfb_t *nsfb, uint8_t pixel)
{
        return nsfb->palette[pixel];
}

static uint8_t
colour_to_pixel(nsfb_t *nsfb, nsfb_colour_t c)
{
        nsfb_colour_t palent;
        int col;

        int dr, dg, db; /* delta red, green blue values */

        int cur_distance;
        int best_distance = INT_MAX;
        uint8_t best_col = 0;

        for (col = 0; col < 256; col++) {
                palent = nsfb->palette[col];

                dr = (c & 0xFF) - (palent & 0xFF);
                dg = ((c >> 8) & 0xFF) - ((palent >> 8) & 0xFF);
                db = ((c >> 16) & 0xFF) - ((palent >> 16) & 0xFF);
                cur_distance = ((dr * dr) + (dg * dg) + (db *db));
                if (cur_distance < best_distance) {
                        best_distance = cur_distance;
                        best_col = col;
                }
        }

        return best_col;
}

#define SIGN(x)  ((x<0) ?  -1  :  ((x>0) ? 1 : 0))

static bool 
line(nsfb_t *nsfb, 
     nsfb_bbox_t *line, 
     int line_width, 
     nsfb_colour_t c, 
     bool dotted, 
     bool dashed)
{
        int w;
        uint8_t ent;
        uint8_t *pvideo;
        int x, y, i;
        int dx, dy, sdy;
        int dxabs, dyabs;

        line_width = line_width;
        dotted = dotted;
        dashed = dashed;

        ent = colour_to_pixel(nsfb, c);

        if (line->y0 == line->y1) {
                /* horizontal line special cased */
                if (!nsfb_plot_clip_ctx(nsfb, line))
                        return true; /* line outside clipping */

                pvideo = get_xy_loc(nsfb, line->x0, line->y0);

                w = line->x1 - line->x0;
                while (w-- > 0) 
                        *(pvideo + w) = ent;
                
                return true;
        } else {
                /* standard bresenham line */
                if (!nsfb_plot_clip_line_ctx(nsfb, line))
                        return true; /* line outside clipping */

                /* the horizontal distance of the line */
                dx = line->x1 - line->x0;
                dxabs = abs (dx);

                /* the vertical distance of the line */
                dy = line->y1 - line->y0;
                dyabs = abs (dy);

                sdy = dx ? SIGN(dy) * SIGN(dx) : SIGN(dy);

                if (dx >= 0)
                        pvideo = get_xy_loc(nsfb, line->x0, line->y0);
                else
                        pvideo = get_xy_loc(nsfb, line->x1, line->y1);

                x = dyabs >> 1;
                y = dxabs >> 1;

                if (dxabs >= dyabs) {
                        /* the line is more horizontal than vertical */
                        for (i = 0; i < dxabs; i++) {
                                *pvideo = ent;

                                pvideo++;
                                y += dyabs;
                                if (y >= dxabs) {
                                        y -= dxabs;
                                        pvideo += sdy * nsfb->linelen;
                                }
                        }
                } else {
                        /* the line is more vertical than horizontal */
                        for (i = 0; i < dyabs; i++) {
                                *pvideo = ent;
                                pvideo += sdy * nsfb->linelen;

                                x += dxabs;
                                if (x >= dyabs) {
                                        x -= dyabs;
                                        pvideo++;
                                }
                        }
                }

        }

	return true;
}

static bool fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c)
{
        int y;
        uint8_t ent;
        uint8_t *pvideo;

        if (!nsfb_plot_clip_ctx(nsfb, rect))
                return true; /* fill lies outside current clipping region */

        pvideo = get_xy_loc(nsfb, rect->x0, rect->y0);

        ent = colour_to_pixel(nsfb, c);

        for (y = rect->y0; y < rect->y1; y++) {
                memset(pvideo, ent, rect->x1 - rect->x0);
                pvideo += nsfb->linelen;
        }

	return true;
}

static bool point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{
        uint8_t *pvideo;

        /* check point lies within clipping region */
        if ((x < nsfb->clip.x0) || 
            (x >= nsfb->clip.x1) ||
            (y < nsfb->clip.y0) ||
            (y >= nsfb->clip.y1)) 
                return true;

        pvideo = get_xy_loc(nsfb, x, y);

        if ((c & 0xFF000000) != 0) {
                if ((c & 0xFF000000) != 0xFF000000) {
                        c = nsfb_plot_ablend(c, pixel_to_colour(nsfb, *pvideo));
                }

                *pvideo = colour_to_pixel(nsfb, c);
        }
	return true;
}

static bool
glyph1(nsfb_t *nsfb,
       nsfb_bbox_t *loc,
       const uint8_t *pixel,
       int pitch,
       nsfb_colour_t c)
{
        uint8_t *pvideo;
        int xloop, yloop;
	int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width = loc->x1 - loc->x0;
        int height = loc->y1 - loc->y0;
        uint8_t fgcol;
        const uint8_t *fntd;
        uint8_t row;

        if (!nsfb_plot_clip_ctx(nsfb, loc))
                return true;

        if (height > (loc->y1 - loc->y0))
                height = (loc->y1 - loc->y0);

        if (width > (loc->x1 - loc->x0))
                width = (loc->x1 - loc->x0);

	xoff = loc->x0 - x;
	yoff = loc->y0 - y;

        pvideo = get_xy_loc(nsfb, loc->x0, loc->y0);

        fgcol = colour_to_pixel(nsfb, c);

        for (yloop = yoff; yloop < height; yloop++) {
                fntd = pixel + (yloop * (pitch>>3)) + (xoff>>3);
                row = (*fntd++) << (xoff & 3);
                for (xloop = xoff; xloop < width ; xloop++) {
                        if (((xloop % 8) == 0) && (xloop != 0)) {
                                row = *fntd++;
                        }

                        if ((row & 0x80) != 0) {
                                *(pvideo + xloop) = fgcol;
                        }
                        row = row << 1;

                }

                pvideo += nsfb->linelen;
        }

	return true;
}

static bool
glyph8(nsfb_t *nsfb,
       nsfb_bbox_t *loc,
       const uint8_t *pixel,
       int pitch,
       nsfb_colour_t c)
{
        uint8_t *pvideo;
        nsfb_colour_t abpixel; /* alphablended pixel */
        int xloop, yloop;
	int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width = loc->x1 - loc->x0;
        int height = loc->y1 - loc->y0;
        uint8_t fgcol;

        if (!nsfb_plot_clip_ctx(nsfb, loc))
                return true;

        if (height > (loc->y1 - loc->y0))
                height = (loc->y1 - loc->y0);

        if (width > (loc->x1 - loc->x0))
                width = (loc->x1 - loc->x0);

	xoff = loc->x0 - x;
	yoff = loc->y0 - y;

        pvideo = get_xy_loc(nsfb, loc->x0, loc->y0);

        fgcol = c & 0xFFFFFF;

        for (yloop = 0; yloop < height; yloop++) {
                for (xloop = 0; xloop < width; xloop++) {
                        abpixel = (pixel[((yoff + yloop) * pitch) + xloop + xoff] << 24) | fgcol;
                        if ((abpixel & 0xFF000000) != 0) {
                                /* pixel is not transparent */
                                if ((abpixel & 0xFF000000) != 0xFF000000) {
                                        abpixel = nsfb_plot_ablend(abpixel,
                                                                   pixel_to_colour(nsfb, *(pvideo + xloop)));
                                }

                                *(pvideo + xloop) = colour_to_pixel(nsfb, abpixel);
                        }
                }
                pvideo += nsfb->linelen;
        }

	return true;
}

static bool 
bitmap(nsfb_t *nsfb,
       const nsfb_bbox_t *loc,
       const nsfb_colour_t *pixel, 
       int bmp_width, 
       int bmp_height, 
       int bmp_stride,
       bool alpha)
{
        uint8_t *pvideo;
        nsfb_colour_t abpixel = 0; /* alphablended pixel */
        int xloop, yloop;
	int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width = loc->x1 - loc->x0;
        int height = loc->y1 - loc->y0;
        nsfb_bbox_t clipped; /* clipped display */

        /* TODO here we should scale the image from bmp_width to width, for
         * now simply crop.
         */
        if (width > bmp_width)
                width = bmp_width;

        if (height > bmp_height)
                height = bmp_height;

        /* The part of the scaled image actually displayed is cropped to the
         * current context.
         */
        clipped.x0 = x;
        clipped.y0 = y;
        clipped.x1 = x + width;
        clipped.y1 = y + height;

        if (!nsfb_plot_clip_ctx(nsfb, &clipped))
                return true;

        if (height > (clipped.y1 - clipped.y0))
                height = (clipped.y1 - clipped.y0);

        if (width > (clipped.x1 - clipped.x0))
                width = (clipped.x1 - clipped.x0);

	xoff = clipped.x0 - x;
	yoff = (clipped.y0 - y) * bmp_width;
	height = height * bmp_stride + yoff;

        /* plot the image */
        pvideo = get_xy_loc(nsfb, clipped.x0, clipped.y0);

        if (alpha) {
                for (yloop = yoff; yloop < height; yloop += bmp_stride) {
                        for (xloop = 0; xloop < width; xloop++) {
                                abpixel = pixel[yloop + xloop + xoff];
                                if ((abpixel & 0xFF000000) != 0) {
                                        if ((abpixel & 0xFF000000) != 0xFF000000) {
                                                abpixel = nsfb_plot_ablend(abpixel,
                                                                           pixel_to_colour(nsfb, *(pvideo + xloop)));
                                        }

                                        *(pvideo + xloop) = colour_to_pixel(nsfb, abpixel);
                                }
                        }
                        pvideo += nsfb->linelen;
                }
        } else {
                for (yloop = yoff; yloop < height; yloop += bmp_stride) {
                        for (xloop = 0; xloop < width; xloop++) {
                                abpixel = pixel[yloop + xloop + xoff];
                                *(pvideo + xloop) = colour_to_pixel(nsfb, abpixel);
                        }
                        pvideo += nsfb->linelen;
                }
        }
	return true;
}






const nsfb_plotter_fns_t _nsfb_8bpp_plotters = {
	.line = line,
	.fill = fill,
        .point = point,
        .bitmap = bitmap,
        .glyph8 = glyph8,
        .glyph1 = glyph1,
};


/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
