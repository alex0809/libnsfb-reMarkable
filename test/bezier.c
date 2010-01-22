/* libnsfb plotetr test program */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#define UNUSED(x) ((x) = (x))


int main(int argc, char **argv)
{
    nsfb_t *nsfb;
    nsfb_event_t event;
    nsfb_bbox_t box;
    nsfb_bbox_t box2;
    uint8_t *fbptr;
    int fbstride;
    nsfb_point_t ctrla;
    nsfb_point_t ctrlb;
    int loop;
    nsfb_plot_pen_t pen;

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

    box2.x0=100;
    box2.y0=100;

    box2.x1=400;
    box2.y1=400;

    pen.stroke_colour = 0xff000000;
    pen.fill_colour = 0xffff0000;
    pen.stroke_type = NFSB_PLOT_OPTYPE_SOLID;
    pen.fill_type = NFSB_PLOT_OPTYPE_NONE;

    for (loop=-300;loop < 600;loop+=100) {
        ctrla.x = 100;
        ctrla.y = loop;

        ctrlb.x = 400;
        ctrlb.y = 500 - loop;

        nsfb_plot_cubic_bezier(nsfb, &box2, &ctrla, &ctrlb, &pen);
    }


    box2.x0=400;
    box2.y0=100;

    box2.x1=600;
    box2.y1=400;

    nsfb_plot_line(nsfb, &box2, &pen);

    box2.x0=800;
    box2.y0=100;

    box2.x1=600;
    box2.y1=400;

    nsfb_plot_line(nsfb, &box2, &pen);

    box2.x0=400;
    box2.y0=100;

    box2.x1=800;
    box2.y1=100;

    ctrla.x = 600;
    ctrla.y = 400;

    pen.stroke_colour = 0xffff0000;

    nsfb_plot_cubic_bezier(nsfb, &box2, &ctrla, &ctrla, &pen);

    box2.x0=400;
    box2.y0=100;

    box2.x1=800;
    box2.y1=100;

    ctrla.x = 600;
    ctrla.y = 400;

    pen.stroke_colour = 0xff0000ff;

    nsfb_plot_quadratic_bezier(nsfb, &box2, &ctrla, &pen);

    nsfb_update(nsfb, &box);
    
    while (event.type != NSFB_EVENT_CONTROL)
        nsfb_event(nsfb, &event, -1);

    return 0;
}
