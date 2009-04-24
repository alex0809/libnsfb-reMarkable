/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported legacy plotter interface for the libnsfb graphics
 * library. This interface should *not* be used for new projects. It is not
 * thread safe as the framebuffer context is held as a global and not passed.
 */

#ifndef _LIBNSFB_LEGACY_PLOT_H
#define _LIBNSFB_LEGACY_PLOT_H 1

/** Set the framebuffer context for all legacy plot operations.
 */
bool nsfb_lplot_ctx(nsfb_t *nsfb);

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
bool nsfb_lplot_clip(int x0, int y0, int x1, int y1);

/** Clears plotting area to a flat colour.
 */
bool nsfb_lplot_clg(nsfb_colour_t c);

/** Plots a rectangle outline. 
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
bool nsfb_lplot_rectangle(int x0, int y0, int width, int height, int line_width, nsfb_colour_t c, bool dotted, bool dashed);

/** Plots a filled rectangle. 
 *
 * Top left corner at (x0,y0), bottom right corner at (x1,y1). Note: (x0,y0) is
 * inside filled area, but (x1,y1) is below and to the right.
 */
bool nsfb_lplot_fill(int x0, int y0, int x1, int y1, nsfb_colour_t c);

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
bool nsfb_lplot_line(int x0, int y0, int x1, int y1, int line_width, nsfb_colour_t c, bool dotted, bool dashed);


/** Plots a filled polygon. 
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 */
bool nsfb_lplot_polygon(const int *p, unsigned int n, nsfb_colour_t fillc);

/** Plots a circle.
 */
bool nsfb_lplot_disc(int x, int y, int radius, nsfb_colour_t c, bool filled);

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
bool nsfb_lplot_arc(int x, int y, int radius, int angle1, int angle2, nsfb_colour_t c);

#endif /* _LIBNSFB_LEGACY_PLOT_H */
