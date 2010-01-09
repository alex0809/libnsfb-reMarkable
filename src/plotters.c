/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2009 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/** \file
 * generic plotter functions which are not depth dependant (implementation).
 */

#include <stdbool.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "nsfb_plot.h"
#include "plotters.h"
#include "frontend.h"

extern const nsfb_plotter_fns_t _nsfb_1bpp_plotters;
extern const nsfb_plotter_fns_t _nsfb_8bpp_plotters;
extern const nsfb_plotter_fns_t _nsfb_16bpp_plotters;
extern const nsfb_plotter_fns_t _nsfb_24bpp_plotters;
extern const nsfb_plotter_fns_t _nsfb_32bpp_plotters;

static bool set_clip(nsfb_t *nsfb, nsfb_bbox_t *clip)
{
    nsfb_bbox_t fbarea;

    /* screen area */
    fbarea.x0 = 0;
    fbarea.y0 = 0;
    fbarea.x1 = nsfb->width;
    fbarea.y1 = nsfb->height;

    if (clip == NULL) {
        nsfb->clip = fbarea;
    } else {
        if (!nsfb_plot_clip(&fbarea, clip))
            return false;

        nsfb->clip = *clip;
    }
    return true;
}

static bool get_clip(nsfb_t *nsfb, nsfb_bbox_t *clip)
{
    *clip = nsfb->clip;
    return true;
}

static bool clg(nsfb_t *nsfb, nsfb_colour_t c)
{
    return nsfb->plotter_fns->fill(nsfb, &nsfb->clip, c);
}

/**
 * Find first filled span along horizontal line at given coordinate
 *
 * \param  p	 array of polygon vertices (x1, y1, x2, y2, ... , xN, yN)
 * \param  n	 number of polygon vertex values (N * 2)
 * \param  x	 current position along current scan line
 * \param  y	 position of current scan line
 * \param  x0	 updated to start of filled area
 * \param  x1	 updated to end of filled area
 * \return true	 if an intersection was found
 */
static bool find_span(const int *p, int n, int x, int y, int *x0, int *x1)
{
	int i;
	int p_x0, p_y0;
	int p_x1, p_y1;
	int x_new;
	bool direction = false;

	*x0 = *x1 = INT_MAX;

	for (i = 0; i < n; i = i + 2) {
		/* get line endpoints */
		if (i != n - 2) {
			/* not the last line */
			p_x0 = p[i];		p_y0 = p[i + 1];
			p_x1 = p[i + 2];	p_y1 = p[i + 3];
		} else {
			/* last line; 2nd endpoint is first vertex */
			p_x0 = p[i];		p_y0 = p[i + 1];
			p_x1 = p[0];		p_y1 = p[1];
		}
		/* ignore horizontal lines */
		if (p_y0 == p_y1)
			continue;

		/* ignore lines that don't cross this y level */
		if ((y < p_y0 && y < p_y1) || (y > p_y0 && y > p_y1))
			continue;

		if (p_x0 == p_x1) {
			/* vertical line, x is constant */
			x_new = p_x0;
		} else {
			/* find intersect */
			x_new = p_x0 + ((long long)(y - p_y0) * (p_x1 - p_x0)) /
					(p_y1 - p_y0);
		}

		/* ignore intersections before current x */
		if (x_new < x)
			continue;

		/* set nearest intersections as filled area endpoints */
		if (x_new < *x0) {
			/* nearer than first endpoint */
			*x1 = *x0;
			*x0 = x_new;
			direction = (p_y0 > p_y1);
		} else if (x_new == *x0) {
			/* same as first endpoint */
			if ((p_y0 > p_y1) != direction)
				*x1 = x_new;
		} else if (x_new < *x1) {
			/* nearer than second endpoint */
			*x1 = x_new;
		}

	}
	if (*x0 == INT_MAX)
		/* no span found */
		return false;

	/* span found */
	if (*x1 == INT_MAX) {
		*x1 = *x0;
		*x0 = x;
		return true;
	}

	return true;
}


/**
 * Plot a polygon
 *
 * \param  nsfb	 framebuffer context
 * \param  p	 array of polygon vertices (x1, y1, x2, y2, ... , xN, yN)
 * \param  n	 number of polygon vertices (N)
 * \param  c	 fill colour
 * \return true	 if no errors
 */
static bool 
polygon(nsfb_t *nsfb, const int *p, unsigned int n, nsfb_colour_t c)
{
	int poly_x0, poly_y0; /* Bounding box top left corner */
	int poly_x1, poly_y1; /* Bounding box bottom right corner */
	int i, j; /* indexes */
	int x0, x1; /* filled span extents */
	int y; /* current y coordinate */
	int y_max; /* bottom of plot area */
	nsfb_bbox_t fline;
	nsfb_plot_pen_t pen;

	/* find no. of vertex values */
	int v = n * 2;

	/* Can't plot polygons with 2 or fewer vertices */
	if (n <= 2)
		return true;

	pen.stroke_colour = c;

	/* Find polygon bounding box */
	poly_x0 = poly_x1 = *p;
	poly_y0 = poly_y1 = p[1];
	for (i = 2; i < v; i = i + 2) {
		j = i + 1;
		if (p[i] < poly_x0)
			poly_x0 = p[i];
		else if (p[i] > poly_x1)
			poly_x1 = p[i];
		if (p[j] < poly_y0)
			poly_y0 = p[j];
		else if (p[j] > poly_y1)
			poly_y1 = p[j];
	}

	/* Don't try to plot it if it's outside the clip rectangle */
	if (nsfb->clip.y1 < poly_y0 ||
            nsfb->clip.y0 > poly_y1 ||
            nsfb->clip.x1 < poly_x0 ||
            nsfb->clip.x0 > poly_x1)
		return true;

	/* Find the top of the important area */
	if (poly_y0 > nsfb->clip.y0)
		y = poly_y0;
	else
		y = nsfb->clip.y0;

	/* Find the bottom of the important area */
	if (poly_y1 < nsfb->clip.y1)
		y_max = poly_y1;
	else
		y_max = nsfb->clip.y1;

	for (; y < y_max; y++) {
		x1 = poly_x0;
		/* For each row */
		while (find_span(p, v, x1, y, &x0, &x1)) {
			/* don't draw anything outside clip region */
			if (x1 < nsfb->clip.x0)
				continue;
			else if (x0 < nsfb->clip.x0)
				x0 = nsfb->clip.x0;
			if (x0 > nsfb->clip.x1)
				break;
			else if (x1 > nsfb->clip.x1)
				x1 = nsfb->clip.x1;

			fline.x0 = x0;
			fline.y0 = y;
			fline.x1 = x1;
			fline.y1 = y;

			/* draw this filled span on current row */
			nsfb->plotter_fns->line(nsfb, 1, &fline, &pen);

			/* don't look for more spans if already at end of clip
			 * region or polygon */
			if (x1 == nsfb->clip.x1 || x1 == poly_x1)
				break;

			if (x0 == x1)
				x1++;
		}
	}
	return true;
}

static bool 
rectangle(nsfb_t *nsfb, nsfb_bbox_t *rect,
	  int line_width, nsfb_colour_t c,
	  bool dotted, bool dashed)
{
    nsfb_bbox_t side[4];
    nsfb_plot_pen_t pen;

    pen.stroke_colour = c;
    pen.stroke_width = line_width;
    if (dotted || dashed) {
	    pen.stroke_type = NFSB_PLOT_OPTYPE_PATTERN;
    } else {
	    pen.stroke_type = NFSB_PLOT_OPTYPE_SOLID;
    }

    side[0] = *rect;
    side[1] = *rect;
    side[2] = *rect;
    side[3] = *rect;

    side[0].y1 = side[0].y0;
    side[1].y0 = side[1].y1;
    side[2].x1 = side[2].x0;
    side[3].x0 = side[3].x1;

    return nsfb->plotter_fns->line(nsfb, 4, side, &pen);
}

/* plotter routine for ellipse points */
static void 
ellipsepoints(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c)
{
    nsfb->plotter_fns->point(nsfb, cx + x, cy + y, c);
    nsfb->plotter_fns->point(nsfb, cx - x, cy + y, c);
    nsfb->plotter_fns->point(nsfb, cx + x, cy - y, c);
    nsfb->plotter_fns->point(nsfb, cx - x, cy - y, c);
}

static void 
ellipsefill(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c)
{
    nsfb_bbox_t fline[2];
    nsfb_plot_pen_t pen;

    pen.stroke_colour = c;

    fline[0].x0 = fline[1].x0 = cx - x;
    fline[0].x1 = fline[1].x1 = cx + x;
    fline[0].y0 = fline[0].y1 = cy + y;
    fline[1].y0 = fline[1].y1 = cy - y;

    nsfb->plotter_fns->line(nsfb, 2, fline, &pen);

}

#define ROUND(a) ((int)(a+0.5))

static bool 
ellipse_midpoint(nsfb_t *nsfb,
		 int cx,
		 int cy,
		 int rx,
		 int ry,
		 nsfb_colour_t c,
		 void (ellipsefn)(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c))
{
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int tworx2 = 2 * rx2;
    int twory2 = 2 * ry2;
    int p;
    int x = 0;
    int y = ry;
    int px = 0;
    int py = tworx2 * y;

    ellipsefn(nsfb, cx, cy, x, y, c);

    /* region 1 */
    p = ROUND(ry2 - (rx2 * ry) + (0.25 * rx2));
    while (px < py) {
        x++;
        px += twory2;
        if (p <0) {
            p+=ry2 + px;
        } else {
            y--;
            py -= tworx2;
            p+=ry2 + px - py;
        }
        ellipsefn(nsfb, cx, cy, x, y, c);
    }

    /* region 2 */
    p = ROUND(ry2*(x+0.5)*(x+0.5) + rx2*(y-1)*(y-1) - rx2*ry2);
    while (y > 0) {
        y--;
        py -= tworx2;
        if (p > 0) {
            p+=rx2 - py;
        } else {
            x++;
            px += twory2;
            p+=rx2 - py + px;
        }
        ellipsefn(nsfb, cx, cy, x, y, c);
    }
    return true;
}


/* plotter routine for 8way circle symetry */
static void 
circlepoints(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c)
{
    nsfb->plotter_fns->point(nsfb, cx + x, cy + y, c);
    nsfb->plotter_fns->point(nsfb, cx - x, cy + y, c);
    nsfb->plotter_fns->point(nsfb, cx + x, cy - y, c);
    nsfb->plotter_fns->point(nsfb, cx - x, cy - y, c);
    nsfb->plotter_fns->point(nsfb, cx + y, cy + x, c);
    nsfb->plotter_fns->point(nsfb, cx - y, cy + x, c);
    nsfb->plotter_fns->point(nsfb, cx + y, cy - x, c);
    nsfb->plotter_fns->point(nsfb, cx - y, cy - x, c);
}

static void 
circlefill(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c)
{
    nsfb_bbox_t fline[4];
    nsfb_plot_pen_t pen;

    pen.stroke_colour = c;

    fline[0].x0 = fline[1].x0 = cx - x;
    fline[0].x1 = fline[1].x1 = cx + x;
    fline[0].y0 = fline[0].y1 = cy + y;
    fline[1].y0 = fline[1].y1 = cy - y;

    fline[2].x0 = fline[3].x0 = cx - y;
    fline[2].x1 = fline[3].x1 = cx + y;
    fline[2].y0 = fline[2].y1 = cy + x;
    fline[3].y0 = fline[3].y1 = cy - x;

    nsfb->plotter_fns->line(nsfb, 4, fline, &pen);
}

static bool circle_midpoint(nsfb_t *nsfb,
                            int cx,
                            int cy,
                            int r,
                            nsfb_colour_t c,
                            void (circfn)(nsfb_t *nsfb, int cx, int cy, int x, int y, nsfb_colour_t c))
{
    int x = 0;
    int y = r;
    int p = 1 - r;

    circfn(nsfb, cx, cy, x, y, c);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        circfn(nsfb, cx, cy, x, y, c);
    }
    return true;
}

static bool ellipse(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{
    int width = (ellipse->x1 - ellipse->x0)>>1;
    int height = (ellipse->y1 - ellipse->y0)>>1;

    if (width == height) {
        /* circle */
        return circle_midpoint(nsfb, ellipse->x0 + width, ellipse->y0 + height, width, c, circlepoints);
    } else {
        return ellipse_midpoint(nsfb, ellipse->x0 + width, ellipse->y0 + height, width, height, c, ellipsepoints);
    }
}

static bool ellipse_fill(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{
    int width = (ellipse->x1 - ellipse->x0) >> 1;
    int height = (ellipse->y1 - ellipse->y0) >> 1;

    if (width == height) {
        /* circle */
        return circle_midpoint(nsfb, ellipse->x0 + width, ellipse->y0 + height, width, c, circlefill);
    } else {
        return ellipse_midpoint(nsfb, ellipse->x0 + width, ellipse->y0 + height, width, height, c, ellipsefill);
    }
}



/* copy an area of screen from one location to another.
 *
 * @warning This implementation is woefully incomplete! 
 */
static bool 
copy(nsfb_t *nsfb, nsfb_bbox_t *srcbox, nsfb_bbox_t *dstbox)
{
    int srcx = srcbox->x0;
    int srcy = srcbox->y0; 
    int dstx = dstbox->x0; 
    int dsty = dstbox->y0;
    int width = dstbox->x1 - dstbox->x0;
    int height = dstbox->y1 - dstbox->y0; 
    uint8_t *srcptr;
    uint8_t *dstptr;
    int hloop;
    nsfb_bbox_t allbox;

    nsfb_plot_add_rect(srcbox, dstbox, &allbox);

    nsfb->frontend_rtns->claim(nsfb, &allbox);

    srcptr = (nsfb->ptr +
              (srcy * nsfb->linelen) +
              ((srcx * nsfb->bpp) / 8));

    dstptr = (nsfb->ptr +
              (dsty * nsfb->linelen) +
              ((dstx * nsfb->bpp) / 8));


    if (width == nsfb->width) {
        /* take shortcut and use memmove */
        memmove(dstptr, srcptr, (width * height * nsfb->bpp) / 8);
    } else {
        if (srcy > dsty) {
            for (hloop = height; hloop > 0; hloop--) {
                memmove(dstptr, srcptr, (width * nsfb->bpp) / 8);
                srcptr += nsfb->linelen;
                dstptr += nsfb->linelen;
            }
        } else {
            srcptr += height * nsfb->linelen;
            dstptr += height * nsfb->linelen;
            for (hloop = height; hloop > 0; hloop--) {
                srcptr -= nsfb->linelen;
                dstptr -= nsfb->linelen;
                memmove(dstptr, srcptr, (width * nsfb->bpp) / 8);
            }
        }
    }

    nsfb->frontend_rtns->update(nsfb, dstbox);

    return true;
}

static bool arc(nsfb_t *nsfb, int x, int y, int radius, int angle1, int angle2, nsfb_colour_t c)
{
        nsfb=nsfb;
        x = x;
        y = y;
        radius = radius;
        c = c;
        angle1=angle1;
        angle2=angle2;
	return true;
}

#define N_SEG 30

static bool 
cubic(nsfb_t *nsfb, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfb_point_t *ctrlb, nsfb_colour_t cl)
{
    nsfb_bbox_t line;
    nsfb_plot_pen_t pen;

    unsigned int seg_loop;
    double t;
    double one_minus_t;
    double a;
    double b;
    double c;
    double d;
    double x;
    double y;

    pen.stroke_colour = cl;

    x = curve->x0;
    y = curve->y0;

    for (seg_loop = 1; seg_loop <= N_SEG; ++seg_loop) {
        t = (double)seg_loop / (double)N_SEG;

        one_minus_t = 1.0 - t;

        a = one_minus_t * one_minus_t * one_minus_t;
        b = 3.0 * t * one_minus_t * one_minus_t;
        c = 3.0 * t * t * one_minus_t;
        d = t * t * t;
 
        line.x0 = x;
        line.y0 = y;

        x = a * curve->x0 + b * ctrla->x + c * ctrlb->x + d * curve->x1;
        y = a * curve->y0 + b * ctrla->y + c * ctrlb->y + d * curve->y1;

        line.x1 = x;
        line.y1 = y;

        nsfb->plotter_fns->line(nsfb, 1, &line, &pen);
    }

    return true;
}

static bool quadratic(nsfb_t *nsfb, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfb_colour_t cl)
{
    nsfb_bbox_t line;
    nsfb_plot_pen_t pen;

    unsigned int seg_loop;
    double t;
    double one_minus_t;
    double a;
    double b;
    double c;
    double x;
    double y;

    pen.stroke_colour = cl;

    x = curve->x0;
    y = curve->y0;

    for (seg_loop = 1; seg_loop <= N_SEG; ++seg_loop) {
        t = (double)seg_loop / (double)N_SEG;

        one_minus_t = 1.0 - t;

        a = one_minus_t * one_minus_t;
        b = 2.0 * t * one_minus_t;
        c = t * t;
 
        line.x0 = x;
        line.y0 = y;

        x = a * curve->x0 + b * ctrla->x + c * curve->x1;
        y = a * curve->y0 + b * ctrla->y + c * curve->y1;

        line.x1 = x;
        line.y1 = y;

        nsfb->plotter_fns->line(nsfb, 1, &line, &pen);
    }

    return true;
}

bool select_plotters(nsfb_t *nsfb)
{
    const nsfb_plotter_fns_t *table = NULL;

    switch (nsfb->bpp) {
        /*    case 1:
        table = &_nsfb_1bpp_plotters;
        break;
        */
    case 8:
        table = &_nsfb_8bpp_plotters;
        break;

    case 16:
        table = &_nsfb_16bpp_plotters;
        break;

        /*
    case 24:
        table = &_nsfb_24bpp_plotters;
        break;
        */
    case 32:
        table = &_nsfb_32bpp_plotters;
        break;

    default:
        return false;
    }

    if (nsfb->plotter_fns != NULL)
        free(nsfb->plotter_fns);

    nsfb->plotter_fns = calloc(1, sizeof(nsfb_plotter_fns_t));
    memcpy(nsfb->plotter_fns, table, sizeof(nsfb_plotter_fns_t));

    /* set the generics */
    nsfb->plotter_fns->clg = clg;
    nsfb->plotter_fns->set_clip = set_clip;
    nsfb->plotter_fns->get_clip = get_clip;
    nsfb->plotter_fns->polygon = polygon;
    nsfb->plotter_fns->rectangle = rectangle;
    nsfb->plotter_fns->ellipse = ellipse;
    nsfb->plotter_fns->ellipse_fill = ellipse_fill;
    nsfb->plotter_fns->copy = copy;
    nsfb->plotter_fns->arc = arc;
    nsfb->plotter_fns->quadratic = quadratic;
    nsfb->plotter_fns->cubic = cubic;

    /* set default clip rectangle to size of framebuffer */
    nsfb->clip.x0 = 0;
    nsfb->clip.y0 = 0;
    nsfb->clip.x1 = nsfb->width;
    nsfb->clip.y1 = nsfb->height;

    return true;
}
