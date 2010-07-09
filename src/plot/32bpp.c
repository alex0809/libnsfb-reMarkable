/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <endian.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "plot.h"


#define UNUSED __attribute__((unused)) 

static inline uint32_t *get_xy_loc(nsfb_t *nsfb, int x, int y)
{
        return (uint32_t *)(nsfb->ptr + (y * nsfb->linelen) + (x << 2));
}

#if __BYTE_ORDER == __BIG_ENDIAN
static inline nsfb_colour_t pixel_to_colour(UNUSED nsfb_t *nsfb, uint32_t pixel)
{
        return (pixel >> 8) & ~0xFF000000U;
}

/* convert a colour value to a 32bpp pixel value ready for screen output */
static inline uint32_t colour_to_pixel(UNUSED nsfb_t *nsfb, nsfb_colour_t c)
{
        return (c << 8);
}
#else /* __BYTE_ORDER == __BIG_ENDIAN */
static inline nsfb_colour_t pixel_to_colour(UNUSED nsfb_t *nsfb, uint32_t pixel)
{
        return ((pixel & 0xFF) << 16) |
                ((pixel & 0xFF00)) |
                ((pixel & 0xFF0000) >> 16);
}

/* convert a colour value to a 32bpp pixel value ready for screen output */
static inline uint32_t colour_to_pixel(UNUSED nsfb_t *nsfb, nsfb_colour_t c)
{
        return ((c & 0xff0000) >> 16) | (c & 0xff00) | ((c & 0xff) << 16);
}
#endif

#define PLOT_TYPE uint32_t
#define PLOT_LINELEN(ll) ((ll) >> 2)

#include "common.c"

static bool fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c)
{
        int w;
        uint32_t *pvid;
        uint32_t ent;
        uint32_t llen;
        uint32_t width;
        uint32_t height;

        if (!nsfb_plot_clip_ctx(nsfb, rect))
                return true; /* fill lies outside current clipping region */

        ent = colour_to_pixel(nsfb, c);
        width = rect->x1 - rect->x0;
        height = rect->y1 - rect->y0;
        llen = (nsfb->linelen >> 2) - width;

        pvid = get_xy_loc(nsfb, rect->x0, rect->y0);

        while (height-- > 0) {
                w = width;
                while (w >= 16) {
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       w-=16;
                }
                while (w >= 4) {
                       *pvid++ = ent; *pvid++ = ent;
                       *pvid++ = ent; *pvid++ = ent;
                       w-=4;
                }
                while (w > 0) {
                       *pvid++ = ent;
                       w--;
                }
                pvid += llen;
        }

        return true;
}

const nsfb_plotter_fns_t _nsfb_32bpp_plotters = {
        .line = line,
        .fill = fill,
        .point = point,
        .bitmap = bitmap,
        .glyph8 = glyph8,
        .glyph1 = glyph1,
        .readrect = readrect,
};

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
