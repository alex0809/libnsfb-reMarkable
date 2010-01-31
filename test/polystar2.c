/* libnsfb ploygon plotter test program */

#define _POSIX_C_SOURCE 199506L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"


#include <time.h>
// Sleep milliseconds
static void sleepMilli(long ms)
{
    const struct timespec ts = {ms / 1000, ms % 1000 * 1000000};
    nanosleep(&ts, NULL);
}

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
    int counter;
    int colour;

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

    radius = (box.x1 / 3);
    sides = 5;
    counter = 0;

    for (counter = 0; counter < 20; counter++) {
        /* claim the whole screen for update */
        nsfb_claim(nsfb, &box);

        nsfb_plot_clg(nsfb, 0xffffffff);

        points = malloc(sizeof(nsfb_point_t) * sides);

        for (loop = 0; loop < sides;loop++) {
            points[(2 * loop) % sides].x = (box.x1 / 2) +
                    (radius * cos(loop * 2 * M_PI / sides));
            points[(2 * loop) % sides].y = (box.y1 / 2) +
                    (radius * sin(loop * 2 * M_PI / sides));
        }

        if (counter % 3 == 0)
            colour = 0xffff0000;
        else if (counter % 3 == 1)
            colour = 0xff00ff00;
        else
            colour = 0xff0000ff;

        nsfb_plot_polygon(nsfb, (const int *)points, sides, colour);
        free(points);

        sides += 2;

        nsfb_update(nsfb, &box);
        sleepMilli(400);
    }

    while (event.type != NSFB_EVENT_CONTROL)
        nsfb_event(nsfb, &event, -1);

    return 0;
}
