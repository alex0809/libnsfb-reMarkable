#ifndef _NSFB_H
#define _NSFB_H 1

#include <stdint.h>


/** NS Framebuffer context
 */
struct nsfb_s {
    int width; /**< Visible width. */
    int height; /**< Visible height. */
    int bpp; /**< Bits per pixel. */

    int refresh; /**< Desired refresh rate for physical displays. */
    char *output_dev; /**> Path to output device for frontends that require it. */

    uint8_t *ptr; /**< Base of video memory. */
    int linelen; /**< length of a video line. */

    nsfb_colour_t palette[256]; /**< palette for index modes */
    nsfb_cursor_t *cursor; /**< cursor */

    struct nsfb_frontend_rtns_s *frontend_rtns; /**< frontend routines. */
    void *frontend_priv; /**< frontend opaque data. */

    nsfb_bbox_t clip; /**< current clipping rectangle for plotters */
    struct nsfb_plotter_fns_s *plotter_fns; /**< Plotter methods */
};


#endif
