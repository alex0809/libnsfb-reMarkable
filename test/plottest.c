#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"

extern const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel; /* 3:RGB, 4:RGBA */
  unsigned char  pixel_data[132 * 135 * 4 + 1];
} nsglobe;

int main(int argc, char **argv)
{
    nsfb_t *nsfb;
    nsfb_bbox_t box;
    nsfb_bbox_t box2;
    nsfb_bbox_t box3;
    uint8_t *fbptr;
    int fbstride;
    int p[] = { 300,300,  350,350, 400,300, 450,250, 400,200};
    int loop;

    nsfb = nsfb_init(NSFB_FRONTEND_SDL);
    if (nsfb == NULL) {
        fprintf(stderr, "Unable to initialise nsfb with SDL frontend\n");
        return 1;
    }

    if (nsfb_init_frontend(nsfb) == -1) {
        fprintf(stderr, "Unable to initialise nsfb frontend\n");
        
    }

    /* get the geometry of the whole screen */
    box.x0 = box.y0 = 0;
    nsfb_get_geometry(nsfb, &box.x1, &box.y1, NULL);

    nsfb_get_framebuffer(nsfb, &fbptr, &fbstride);

    /* claim the whole screen for update */
    nsfb_claim(nsfb, &box);

    /* first test, repeatedly clear the graphics area, should result in teh
     * same operation as a single clear to the final colour 
     */
    for (loop = 0; loop < 256;loop++) {
        nsfb_plot_clg(nsfb, 0xffffff00 | loop);
    }

    /* draw radial lines from the origin */
    for (loop = 0; loop < box.x1; loop += 20) {
        box2 = box;
        box2.x1 = loop;
        nsfb_plot_line(nsfb, &box2, 1, 0xff000000, false, false);
    }
    
    /* draw radial lines from the bottom right */
    for (loop = 0; loop < box.x1; loop += 20) {
        box2 = box;
        box2.x0 = loop;
        nsfb_plot_line(nsfb, &box2, 1, 0xffff0000, false, false);
    }
    
    /* draw radial lines from the bottom left */
    for (loop = 0; loop < box.x1; loop += 20) {
        box2.x0 = box.x0;
        box2.x1 = loop;
        box2.y0 = box.y1;
        box2.y1 = box.y0;
        nsfb_plot_line(nsfb, &box2, 1, 0xff00ff00, false, false);
    }

    /* draw radial lines from the top right */
    for (loop = 0; loop < box.x1; loop += 20) {
        box2.x0 = box.x1;
        box2.x1 = loop;
        box2.y0 = box.y0;
        box2.y1 = box.y1;
        nsfb_plot_line(nsfb, &box2, 1, 0xff0000ff, false, false);
    }

    /* draw an unclipped rectangle */
    box2.x0 = box2.y0 = 100;
    box2.x1 = box2.y1 = 300;

    nsfb_plot_rectangle_fill(nsfb, &box2, 0xff0000ff);

    nsfb_plot_rectangle(nsfb, &box2, 1, 0xff00ff00, false, false);

    nsfb_plot_polygon(nsfb, p, 5, 0xffff0000);

    nsfb_plot_set_clip(nsfb, &box2);

    box3.x0 = box3.y0 = 200;
    box3.x1 = box3.y1 = 400;

    nsfb_plot_rectangle_fill(nsfb, &box3, 0xff00ffff);

    nsfb_plot_rectangle(nsfb, &box3, 1, 0xffffff00, false, false);

    for (loop = 100; loop < 400;loop++) {
        nsfb_plot_point(nsfb, loop, 150, 0xffaa1111);
        nsfb_plot_point(nsfb, loop, 160, 0x99aa1111);
    }

    nsfb_plot_set_clip(nsfb, NULL);

    box3.x0 = box3.y0 = 400;
    box3.x1 = box3.y1 = 600;

    nsfb_plot_ellipse_fill(nsfb, &box3, 0xffff0000);

    nsfb_plot_ellipse(nsfb, &box3, 0xff0000ff);

    box3.x0 = 500;
    box3.x1 = 700;
    box3.y0 = 400;
    box3.y1 = 500;

    nsfb_plot_ellipse_fill(nsfb, &box3, 0xffff0000);

    nsfb_plot_ellipse(nsfb, &box3, 0xff0000ff);

    box3.x0 = 600;
    box3.x1 = 700;
    box3.y0 = 300;
    box3.y1 = 500;

    nsfb_plot_ellipse_fill(nsfb, &box3, 0xff0000ff);

    nsfb_plot_ellipse(nsfb, &box3, 0xffff0000);

    nsfb_plot_copy(nsfb,400,400,100,100,600,200);

    box3.x0 = 50;
    box3.x1 = 200;
    box3.y0 = 300;
    box3.y1 = 500;

    nsfb_plot_bitmap(nsfb, &box3, (nsfb_colour_t *)nsglobe.pixel_data, nsglobe.width, nsglobe.height, nsglobe.width, true);

    nsfb_release(nsfb, &box);

    /* random rectangles in clipped area*/
    box2.x0 = 400;
    box2.y0 = 50;
    box2.x1 = 600;
    box2.y1 = 100;

    nsfb_plot_set_clip(nsfb, &box2);

    srand(1234);

    for (loop=0; loop < 10000; loop++) {
    nsfb_claim(nsfb, &box2);
        box3.x0 = rand() / (RAND_MAX / box.x1);
        box3.y0 = rand() / (RAND_MAX / box.y1);
        box3.x1 = rand() / (RAND_MAX / 400);
        box3.y1 = rand() / (RAND_MAX / 400);
        nsfb_plot_rectangle_fill(nsfb, &box3, 0xff000000 | rand());
    nsfb_release(nsfb, &box2);
    }


    while (true )
        nsfb_input(nsfb);

    return 0;
}
