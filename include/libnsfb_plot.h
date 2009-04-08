/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
bool nsfb_plot_set_clip(nsfb_t *nsfb, nsfb_bbox_t *clip);

/** Clears plotting area to a flat colour.
 */
bool nsfb_plot_clg(nsfb_t *nsfb, nsfb_colour_t c);

/** Plots a rectangle outline. 
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
bool nsfb_plot_rectangle(nsfb_t *nsfb, nsfb_bbox_t *rect, int line_width, nsfb_colour_t c, bool dotted, bool dashed);

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
bool nsfb_plot_rectangle_fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c);

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
bool nsfb_plot_line(nsfb_t *nsfb, nsfb_bbox_t *line, int line_width, nsfb_colour_t c, bool dotted, bool dashed);

/** Plots a filled polygon. 
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
bool nsfb_plot_polygon(nsfb_t *nsfb, const int *p, unsigned int n, nsfb_colour_t fill);

/** Plot an ellipse.
 */
bool nsfb_plot_ellipse(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c);

/** Plot a filled ellipse.
 */
bool nsfb_plot_ellipse_fill(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c);

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
bool nsfb_plot_arc(nsfb_t *nsfb, int x, int y, int radius, int angle1, int angle2, nsfb_colour_t c);

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
bool nsfb_plot_point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c);

/** copy an area of screen 
 *
 * Copy an area of the display.
 */
bool nsfb_plot_copy(nsfb_t *nsfb, int srcx, int srcy, int width, int height, int dstx, int dsty);

/** Plot bitmap.
 */
bool nsfb_plot_bitmap(nsfb_t *nsfb, nsfb_bbox_t *loc,  nsfb_colour_t *pixel, int bmp_width, int bmp_height, int bmp_stride, bool alpha);
