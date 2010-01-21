/* libnsfb ploygon plotter test program */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"

#define UNUSED(x) ((x) = (x))


int main(int argc, char **argv)
{
    nsfb_t *nsfb;
    nsfb_event_t event;
    nsfb_bbox_t box;
    uint8_t *fbptr;
    int fbstride;

    int sides;
    int radius;
    nsfb_point_t *points;
    int loop;
//    nsfb_plot_pen_t pen;

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

    radius = (box.x1 / 3);

    for (sides = 13; sides >=3; sides--) {
	    points = malloc(sizeof(nsfb_point_t) * sides);

	    for (loop = 0; loop < sides;loop++) {
		    points[loop].x = (box.x1 / 2) + 
			    (radius * cos(loop * 2 * M_PI / sides));
		    points[loop].y = (box.y1 / 2) + 
			    (radius * sin(loop * 2 * M_PI / sides));
	    }

	    nsfb_plot_polygon(nsfb, (const int *)points, sides, 
			      0xff000000 | (0xffffff / (sides * 2)));

	    free(points);
	    radius -= 25;
    }

    nsfb_update(nsfb, &box);
    
    while (event.type != NSFB_EVENT_CONTROL)
        nsfb_event(nsfb, &event, -1);

    return 0;
}
