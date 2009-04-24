/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported plotter interface for the libnsfb graphics library.
 */

#include <stdbool.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_legacy_plot.h"
#include "nsfb.h"
#include "nsfb_plot.h"

/* legacy interface global context */
static nsfb_t *gnsfb;

bool nsfb_lplot_ctx(nsfb_t *nsfb)
{
    gnsfb = nsfb;
    return true;
}

bool nsfb_lplot_clip(int x0, int y0, int x1, int y1)
{
    nsfb_bbox_t clip;
    clip.x0 = x0;
    clip.y0 = y0;
    clip.x1 = x1;
    clip.y1 = y1;

    return gnsfb->plotter_fns->set_clip(gnsfb, &clip);
}

bool nsfb_lplot_line(int x0, int y0, int x1, int y1, int line_width,
			nsfb_colour_t c, bool dotted, bool dashed)
{
    nsfb_bbox_t line;
    line.x0 = x0;
    line.y0 = y0;
    line.x1 = x1;
    line.y1 = y1;
    return gnsfb->plotter_fns->line(gnsfb, &line, line_width, c, dotted, dashed);
}

bool nsfb_lplot_rectangle(int x0, 
                               int y0, 
                               int width, 
                               int height,
                               int line_width, 
                               nsfb_colour_t c, 
                               bool dotted, 
                               bool dashed)
{
    nsfb_bbox_t rect;
    rect.x0 = x0;
    rect.y0 = y0;
    rect.x1 = x0 + width;
    rect.y1 = y0 + height;

    return gnsfb->plotter_fns->rectangle(gnsfb, &rect, line_width, c, dotted, dashed);

}

bool nsfb_lplot_polygon(const int *p, unsigned int n, nsfb_colour_t fillc)
{
    return gnsfb->plotter_fns->polygon(gnsfb, p, n, fillc);
}

bool nsfb_lplot_fill(int x0, int y0, int x1, int y1, nsfb_colour_t c)
{
    nsfb_bbox_t rect;
    rect.x0 = x0;
    rect.y0 = y0;
    rect.x1 = x1;
    rect.y1 = y1;

    return gnsfb->plotter_fns->fill(gnsfb, &rect, c);
}

bool nsfb_lplot_clg(nsfb_colour_t c)
{
    return gnsfb->plotter_fns->clg(gnsfb, c);
}


bool 
nsfb_lplot_disc(int x, int y, int radius, nsfb_colour_t c, bool filled)
{
    nsfb_bbox_t ellipse;
    ellipse.x0 = x - radius;
    ellipse.y0 = y - radius;
    ellipse.x1 = x + radius;
    ellipse.y1 = y + radius;

    if (filled)
        return gnsfb->plotter_fns->ellipse_fill(gnsfb, &ellipse, c);
    else
        return gnsfb->plotter_fns->ellipse(gnsfb, &ellipse, c);
}

bool 
nsfb_lplot_arc(int x, int y, int radius, int angle1, int angle2,
	    		nsfb_colour_t c)
{
    return gnsfb->plotter_fns->arc(gnsfb, x, y, radius, angle1, angle2, c);
}



