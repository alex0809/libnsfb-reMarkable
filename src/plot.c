/* public plotter interface */

#include <stdbool.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "nsfb.h"
#include "nsfb_plot.h"

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
bool nsfb_plot_set_clip(nsfb_t *nsfb, nsfb_bbox_t *clip)
{
    return nsfb->plotter_fns->clip(nsfb, clip);
}

/** Clears plotting area to a flat colour.
 */
bool nsfb_plot_clg(nsfb_t *nsfb, nsfb_colour_t c)
{
    return nsfb->plotter_fns->clg(nsfb, c);
}

/** Plots a rectangle outline. 
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
bool 
nsfb_plot_rectangle(nsfb_t *nsfb, 
                    nsfb_bbox_t *rect, 
                    int line_width, 
                    nsfb_colour_t c, 
                    bool dotted, 
                    bool dashed)
{
    return nsfb->plotter_fns->rectangle(nsfb, rect, line_width, c, dotted, dashed);

}

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
bool nsfb_plot_rectangle_fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c)
{
    return nsfb->plotter_fns->fill(nsfb, rect, c);
}

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
bool nsfb_plot_line(nsfb_t *nsfb, nsfb_bbox_t *line, int line_width, nsfb_colour_t c, bool dotted, bool dashed)
{
    return nsfb->plotter_fns->line(nsfb, line, line_width, c, dotted, dashed);
}

/** Plots a filled polygon. 
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
bool nsfb_plot_polygon(nsfb_t *nsfb, const int *p, unsigned int n, nsfb_colour_t fill)
{
    return nsfb->plotter_fns->polygon(nsfb, p, n, fill);
}

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
bool nsfb_plot_arc(nsfb_t *nsfb, int x, int y, int radius, int angle1, int angle2, nsfb_colour_t c)
{
    return nsfb->plotter_fns->arc(nsfb, x, y, radius, angle1, angle2, c);
}

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
bool nsfb_plot_point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{
    return nsfb->plotter_fns->point(nsfb, x, y, c);
}

bool nsfb_plot_ellipse(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{
    return nsfb->plotter_fns->ellipse(nsfb, ellipse, c);
}

bool nsfb_plot_ellipse_fill(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{
    return nsfb->plotter_fns->ellipse_fill(nsfb, ellipse, c);
}

bool nsfb_plot_copy(nsfb_t *nsfb, int srcx, int srcy, int width, int height, int dstx, int dsty)
{
    return nsfb->plotter_fns->copy(nsfb, srcx, srcy, width, height, dstx, dsty);
}

bool nsfb_plot_bitmap(nsfb_t *nsfb, nsfb_bbox_t *loc, const nsfb_colour_t *pixel, int bmp_width, int bmp_height, int bmp_stride, bool alpha)
{
    return nsfb->plotter_fns->bitmap(nsfb, loc,  pixel, bmp_width, bmp_height, bmp_stride, alpha);
}

/** Plot an 8 bit glyph.
 */
bool nsfb_plot_glyph8(nsfb_t *nsfb, nsfb_bbox_t *loc, const uint8_t *pixel, int pitch, nsfb_colour_t c)
{
    return nsfb->plotter_fns->glyph8(nsfb, loc, pixel, pitch, c);
}


/** Plot an 1 bit glyph.
 */
bool nsfb_plot_glyph1(nsfb_t *nsfb, nsfb_bbox_t *loc, const uint8_t *pixel, int pitch, nsfb_colour_t c)
{
    return nsfb->plotter_fns->glyph1(nsfb, loc, pixel, pitch, c);
}
