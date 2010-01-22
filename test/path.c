/* libnsfb plotetr test program */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#define UNUSED(x) ((x) = (x))

#define PENT(op, xco, yco) path[count].operation = op;  \
    path[count].point.x = (xco);                        \
    path[count].point.y = (yco);                        \
    count++

static int fill_shape(nsfb_plot_pathop_t *path, int xoff, int yoff) 
{
    int count = 0;

    PENT(NFSB_PLOT_PATHOP_MOVE, xoff, yoff);
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 100, yoff + 100);
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 100, yoff );
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 200, yoff + 100);
    PENT(NFSB_PLOT_PATHOP_MOVE, xoff + 200, yoff - 200);
    PENT(NFSB_PLOT_PATHOP_MOVE, xoff + 300, yoff + 300);
    PENT(NFSB_PLOT_PATHOP_CUBIC, xoff + 300, yoff );
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 400, yoff + 100);
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 400, yoff );
    PENT(NFSB_PLOT_PATHOP_MOVE, xoff + 500, yoff + 200);
    PENT(NFSB_PLOT_PATHOP_QUAD, xoff + 500, yoff );
    PENT(NFSB_PLOT_PATHOP_LINE, xoff + 600, yoff + 150);
    PENT(NFSB_PLOT_PATHOP_LINE, xoff, yoff + 150);
    PENT(NFSB_PLOT_PATHOP_LINE, xoff, yoff);

    return count;
}

int main(int argc, char **argv)
{
    nsfb_t *nsfb;
    nsfb_event_t event;
    nsfb_bbox_t box;
    uint8_t *fbptr;
    int fbstride;
    nsfb_plot_pen_t pen;
    nsfb_plot_pathop_t path[20];

    UNUSED(argc);
    UNUSED(argv);

    nsfb = nsfb_init(NSFB_FRONTEND_SDL);
    if (nsfb == NULL) {
        fprintf(stderr, "Unable to initialise nsfb with SDL frontend\n");
        return 1;
    }

    if (nsfb_init_frontend(nsfb) == -1) {
        fprintf(stderr, "Unable to initialise nsfb frontend\n");
        return 2;
    }

    /* get the geometry of the whole screen */
    box.x0 = box.y0 = 0;
    nsfb_get_geometry(nsfb, &box.x1, &box.y1, NULL);

    nsfb_get_framebuffer(nsfb, &fbptr, &fbstride);

    /* claim the whole screen for update */
    nsfb_claim(nsfb, &box);

    nsfb_plot_clg(nsfb, 0xffffffff);

    pen.stroke_colour = 0xff0000ff;
    pen.fill_colour = 0xffff0000;
    pen.stroke_type = NFSB_PLOT_OPTYPE_SOLID;
    pen.fill_type = NFSB_PLOT_OPTYPE_NONE;

    nsfb_plot_path(nsfb, fill_shape(path, 100, 100), path, &pen);

    pen.fill_type = NFSB_PLOT_OPTYPE_SOLID;

    nsfb_plot_path(nsfb, fill_shape(path, 100, 300), path, &pen);

    nsfb_update(nsfb, &box);
    
    while (event.type != NSFB_EVENT_CONTROL)
        nsfb_event(nsfb, &event, -1);

    return 0;
}
