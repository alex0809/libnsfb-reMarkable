/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "frontend.h"
#include "plot.h"
#include "cursor.h"

#if defined(NEED_HINTS_ALLOC)
xcb_size_hints_t *
xcb_alloc_size_hints(void)
{
    return calloc(1, sizeof(xcb_size_hints_t));
}

void
xcb_free_size_hints(xcb_size_hints_t *hints)
{
    free(hints);
}
#endif

#define X_BUTTON_LEFT 1
#define X_BUTTON_MIDDLE 2
#define X_BUTTON_RIGHT 3
#define X_BUTTON_WHEELUP 4
#define X_BUTTON_WHEELDOWN 5

typedef struct xstate_s {
    xcb_connection_t *connection; /* The x server connection */
    xcb_screen_t *screen; /* The screen to put the window on */

    xcb_shm_segment_info_t shminfo;

    xcb_image_t *image; /* The X image buffer */

    xcb_window_t window; /* The handle to the window */
    xcb_pixmap_t pmap; /* The handle to the backing pixmap */
    xcb_gcontext_t gc; /* The handle to the pixmap plotting graphics context */
    xcb_shm_seg_t segment; /* The handle to the image shared memory */
} xstate_t;

enum nsfb_key_code_e x_nsfb_map[] = {
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_BACKSPACE,
    NSFB_KEY_TAB,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_CLEAR,
    NSFB_KEY_RETURN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_PAUSE,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_ESCAPE,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_SPACE,
    NSFB_KEY_EXCLAIM,
    NSFB_KEY_QUOTEDBL,
    NSFB_KEY_HASH,
    NSFB_KEY_DOLLAR,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_AMPERSAND,
    NSFB_KEY_QUOTE,
    NSFB_KEY_LEFTPAREN,
    NSFB_KEY_RIGHTPAREN,
    NSFB_KEY_ASTERISK,
    NSFB_KEY_PLUS,
    NSFB_KEY_COMMA,
    NSFB_KEY_MINUS,
    NSFB_KEY_PERIOD,
    NSFB_KEY_SLASH,
    NSFB_KEY_0,
    NSFB_KEY_1,
    NSFB_KEY_2,
    NSFB_KEY_3,
    NSFB_KEY_4,
    NSFB_KEY_5,
    NSFB_KEY_6,
    NSFB_KEY_7,
    NSFB_KEY_8,
    NSFB_KEY_9,
    NSFB_KEY_COLON,
    NSFB_KEY_SEMICOLON,
    NSFB_KEY_LESS,
    NSFB_KEY_EQUALS,
    NSFB_KEY_GREATER,
    NSFB_KEY_QUESTION,
    NSFB_KEY_AT,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_LEFTBRACKET,
    NSFB_KEY_BACKSLASH,
    NSFB_KEY_RIGHTBRACKET,
    NSFB_KEY_CARET,
    NSFB_KEY_UNDERSCORE,
    NSFB_KEY_BACKQUOTE,
    NSFB_KEY_a,
    NSFB_KEY_b,
    NSFB_KEY_c,
    NSFB_KEY_d,
    NSFB_KEY_e,
    NSFB_KEY_f,
    NSFB_KEY_g,
    NSFB_KEY_h,
    NSFB_KEY_i,
    NSFB_KEY_j,
    NSFB_KEY_k,
    NSFB_KEY_l,
    NSFB_KEY_m,
    NSFB_KEY_n,
    NSFB_KEY_o,
    NSFB_KEY_p,
    NSFB_KEY_q,
    NSFB_KEY_r,
    NSFB_KEY_s,
    NSFB_KEY_t,
    NSFB_KEY_u,
    NSFB_KEY_v,
    NSFB_KEY_w,
    NSFB_KEY_x,
    NSFB_KEY_y,
    NSFB_KEY_z,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_DELETE,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_KP0,
    NSFB_KEY_KP1,
    NSFB_KEY_KP2,
    NSFB_KEY_KP3,
    NSFB_KEY_KP4,
    NSFB_KEY_KP5,
    NSFB_KEY_KP6,
    NSFB_KEY_KP7,
    NSFB_KEY_KP8,
    NSFB_KEY_KP9,
    NSFB_KEY_KP_PERIOD,
    NSFB_KEY_KP_DIVIDE,
    NSFB_KEY_KP_MULTIPLY,
    NSFB_KEY_KP_MINUS,
    NSFB_KEY_KP_PLUS,
    NSFB_KEY_KP_ENTER,
    NSFB_KEY_KP_EQUALS,
    NSFB_KEY_UP,
    NSFB_KEY_DOWN,
    NSFB_KEY_RIGHT,
    NSFB_KEY_LEFT,
    NSFB_KEY_INSERT,
    NSFB_KEY_HOME,
    NSFB_KEY_END,
    NSFB_KEY_PAGEUP,
    NSFB_KEY_PAGEDOWN,
    NSFB_KEY_F1,
    NSFB_KEY_F2,
    NSFB_KEY_F3,
    NSFB_KEY_F4,
    NSFB_KEY_F5,
    NSFB_KEY_F6,
    NSFB_KEY_F7,
    NSFB_KEY_F8,
    NSFB_KEY_F9,
    NSFB_KEY_F10,
    NSFB_KEY_F11,
    NSFB_KEY_F12,
    NSFB_KEY_F13,
    NSFB_KEY_F14,
    NSFB_KEY_F15,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_NUMLOCK,
    NSFB_KEY_CAPSLOCK,
    NSFB_KEY_SCROLLOCK,
    NSFB_KEY_RSHIFT,
    NSFB_KEY_LSHIFT,
    NSFB_KEY_RCTRL,
    NSFB_KEY_LCTRL,
    NSFB_KEY_RALT,
    NSFB_KEY_LALT,
    NSFB_KEY_RMETA,
    NSFB_KEY_LMETA,
    NSFB_KEY_LSUPER,
    NSFB_KEY_RSUPER,
    NSFB_KEY_MODE,
    NSFB_KEY_COMPOSE,
    NSFB_KEY_HELP,
    NSFB_KEY_PRINT,
    NSFB_KEY_SYSREQ,
    NSFB_KEY_BREAK,
    NSFB_KEY_MENU,
    NSFB_KEY_POWER,
    NSFB_KEY_EURO,
    NSFB_KEY_UNDO,
};

/*
  static void
  set_palette(nsfb_t *nsfb)
  {
  X_Surface *x_screen = nsfb->frontend_priv;
  X_Color palette[256];
  int rloop, gloop, bloop;
  int loop = 0;

  // build a linear R:3 G:3 B:2 colour cube palette.
  for (rloop = 0; rloop < 8; rloop++) {
  for (gloop = 0; gloop < 8; gloop++) {
  for (bloop = 0; bloop < 4; bloop++) {
  palette[loop].r = (rloop << 5) | (rloop << 2) | (rloop >> 1);
  palette[loop].g = (gloop << 5) | (gloop << 2) | (gloop >> 1);
  palette[loop].b = (bloop << 6) | (bloop << 4) | (bloop << 2) | (bloop);
  nsfb->palette[loop] = palette[loop].r |
  palette[loop].g << 8 |
  palette[loop].b << 16;
  loop++;
  }
  }
  }

  // Set palette
  //X_SetColors(x_screen, palette, 0, 256);

  }
*/

static int
update_and_redraw_pixmap(xstate_t *xstate, int x, int y, int width, int height)
{
    if (xstate->shminfo.shmseg == 0) {
        /* not using shared memory */
        xcb_put_image(xstate->connection,
                      xstate->image->format,
                      xstate->pmap,
                      xstate->gc,
                      xstate->image->width,
                      height,
                      0,
                      y,
                      0,
                      xstate->image->depth,
                      (height) * xstate->image->stride,
                      xstate->image->data + (y * xstate->image->stride));
    } else {
        /* shared memory */
        xcb_image_shm_put(xstate->connection, xstate->pmap, xstate->gc, xstate->image, xstate->shminfo, x,y,x,y,width,height,0);
        xcb_flush(xstate->connection);
    }

    xcb_copy_area(xstate->connection,
                  xstate->pmap,
                  xstate->window,
                  xstate->gc,
                  x, y,
                  x, y,
                  width, height);
    return 0;
}

static int x_set_geometry(nsfb_t *nsfb, int width, int height, int bpp)
{
    if (nsfb->frontend_priv != NULL)
        return -1; /* if were already initialised fail */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->bpp = bpp;

    /* select default sw plotters for bpp */
    select_plotters(nsfb);

    return 0;
}


static xcb_format_t *
find_format(xcb_connection_t * c, uint8_t depth, uint8_t bpp)
{
    const xcb_setup_t *setup = xcb_get_setup(c);
    xcb_format_t *fmt = xcb_setup_pixmap_formats(setup);
    xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
    for(; fmt != fmtend; ++fmt)
        if((fmt->depth == depth) && (fmt->bits_per_pixel == bpp)) {
            return fmt;
        }
    return 0;
}

static xcb_image_t *
create_shm_image(xstate_t *xstate, int width, int height, int bpp)
{
    const xcb_setup_t *setup = xcb_get_setup(xstate->connection);
    unsigned char *image_data;
    xcb_format_t *fmt;
    int depth = bpp;
    uint32_t image_size;
    int shmid;

    xcb_shm_query_version_reply_t *rep;
    xcb_shm_query_version_cookie_t ck;

    ck = xcb_shm_query_version(xstate->connection);
    rep = xcb_shm_query_version_reply(xstate->connection, ck , NULL);
    if ((!rep) ||
        (rep->major_version < 1) ||
        (rep->major_version == 1 && rep->minor_version == 0)) {
        fprintf (stderr, "No or insufficient shm support...\n");
        return NULL;
    }
    free(rep);

    if (bpp == 32)
        depth = 24;

    fmt = find_format(xstate->connection, depth, bpp);
    if (fmt == NULL)
        return NULL;

    /* doing it this way ensures we deal with bpp smaller than 8 */
    image_size = (bpp * width * height) >> 3;

    /* get the shared memory segment */
    shmid = shmget(IPC_PRIVATE, image_size, IPC_CREAT|0777);
    if (shmid == -1)
        return NULL;

    xstate->shminfo.shmid = shmid;
    xstate->shminfo.shmaddr = shmat(xstate->shminfo.shmid, 0, 0);
    image_data = xstate->shminfo.shmaddr;

    xstate->shminfo.shmseg = xcb_generate_id(xstate->connection);
    xcb_shm_attach(xstate->connection,
                   xstate->shminfo.shmseg,
                   xstate->shminfo.shmid, 0);

    shmctl(xstate->shminfo.shmid, IPC_RMID, 0);

    return xcb_image_create(width,
                            height,
                            XCB_IMAGE_FORMAT_Z_PIXMAP,
                            fmt->scanline_pad,
                            fmt->depth,
                            fmt->bits_per_pixel,
                            0,
                            setup->image_byte_order,
                            XCB_IMAGE_ORDER_LSB_FIRST,
                            image_data,
                            image_size,
                            image_data);
}


static xcb_image_t *
create_image(xcb_connection_t *c, int width, int height, int bpp)
{
    const xcb_setup_t *setup = xcb_get_setup(c);
    unsigned char *image_data;
    xcb_format_t *fmt;
    int depth = bpp;
    uint32_t image_size;

    if (bpp == 32)
        depth = 24;

    fmt = find_format(c, depth, bpp);
    if (fmt == NULL)
        return NULL;

    /* doing it this way ensures we deal with bpp smaller than 8 */
    image_size = (bpp * width * height) >> 3;

    image_data = calloc(1, image_size);
    if (image_data == NULL)
        return NULL;

    return xcb_image_create(width,
                            height,
                            XCB_IMAGE_FORMAT_Z_PIXMAP,
                            fmt->scanline_pad,
                            fmt->depth,
                            fmt->bits_per_pixel,
                            0,
                            setup->image_byte_order,
                            XCB_IMAGE_ORDER_LSB_FIRST,
                            image_data,
                            image_size,
                            image_data);
}

/**
 * Create a blank cursor.
 * The empty pixmaps is leaked.
 *
 * @param conn xcb connection
 * @param scr xcb XCB screen
 */
static xcb_cursor_t
create_blank_cursor(xcb_connection_t *conn, const xcb_screen_t *scr)
{
    xcb_cursor_t cur = xcb_generate_id(conn);
    xcb_pixmap_t pix = xcb_generate_id(conn);
    xcb_void_cookie_t ck;
    xcb_generic_error_t *err;

    ck = xcb_create_pixmap_checked (conn, 1, pix, scr->root, 1, 1);
    err = xcb_request_check (conn, ck);
    if (err) {
        fprintf (stderr, "Cannot create pixmap: %d", err->error_code);
        free (err);
    }
    ck = xcb_create_cursor_checked (conn, cur, pix, pix, 0, 0, 0, 0, 0, 0, 0, 0);
    err = xcb_request_check (conn, ck);
    if (err) {
        fprintf (stderr, "Cannot create cursor: %d", err->error_code);
        free (err);
    }
    return cur;
}


static int x_initialise(nsfb_t *nsfb)
{
    uint32_t mask;
    uint32_t values[3];
    xcb_size_hints_t *hints;
    xstate_t *xstate = nsfb->frontend_priv;
    xcb_cursor_t blank_cursor;

    if (xstate != NULL)
        return -1; /* already initialised */

    /* sanity check bpp. */
    if ((nsfb->bpp != 32) && (nsfb->bpp != 16) && (nsfb->bpp != 8))
        return -1;

    xstate = calloc(1, sizeof(xstate_t));
    if (xstate == NULL)
        return -1; /* no memory */

    /* open connection with the server */
    xstate->connection = xcb_connect (NULL, NULL);
    if (xstate->connection == NULL) {
        fprintf(stderr, "Unable to open display\n");
        free(xstate);
    }

    /* get screen */
    xstate->screen = xcb_setup_roots_iterator(xcb_get_setup(xstate->connection)).data;

    /* create image */
    xstate->image = create_shm_image(xstate, nsfb->width, nsfb->height, nsfb->bpp);

    if (xstate->image == NULL)
        xstate->image = create_image(xstate->connection, nsfb->width, nsfb->height, nsfb->bpp);

    if (xstate->image == NULL) {
        fprintf(stderr, "Unable to create image\n");
        free(xstate);
        xcb_disconnect(xstate->connection);
        return -1;
    }

    /* ensure plotting information is stored */
    nsfb->frontend_priv = xstate;
    nsfb->ptr = xstate->image->data;
    nsfb->linelen = xstate->image->stride;

    /* get blank cursor */
    blank_cursor = create_blank_cursor(xstate->connection, xstate->screen);

    /* create window */
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_CURSOR;
    values[0] = xstate->screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE |
                XCB_EVENT_MASK_POINTER_MOTION;
    values[2] = blank_cursor;

    xstate->window = xcb_generate_id(xstate->connection);
    xcb_create_window (xstate->connection,
                       XCB_COPY_FROM_PARENT,
                       xstate->window,
                       xstate->screen->root,
                       10, 10, xstate->image->width, xstate->image->height, 1,
                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
                       xstate->screen->root_visual,
                       mask, values);
    /* set size hits on window */
    hints = xcb_alloc_size_hints();
    xcb_size_hints_set_max_size(hints, xstate->image->width, xstate->image->height);
    xcb_size_hints_set_min_size(hints, xstate->image->width, xstate->image->height);
    xcb_set_wm_size_hints(xstate->connection, xstate->window, WM_NORMAL_HINTS, hints);
    xcb_free_size_hints(hints);

    /* create backing pixmap */
    xstate->pmap = xcb_generate_id(xstate->connection);
    xcb_create_pixmap(xstate->connection, 24, xstate->pmap, xstate->window, xstate->image->width, xstate->image->height);

    /* create pixmap plot gc */
    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    values[0] = xstate->screen->black_pixel;
    values[1] = 0xffffff;

    xstate->gc = xcb_generate_id (xstate->connection);
    xcb_create_gc(xstate->connection, xstate->gc, xstate->pmap, mask, values);

    /*    if (nsfb->bpp == 8)
          set_palette(nsfb);
    */

    /* put the image into the pixmap */
    update_and_redraw_pixmap(xstate, 0, 0, xstate->image->width, xstate->image->height);


    /* show the window */
    xcb_map_window (xstate->connection, xstate->window);
    xcb_flush(xstate->connection);

    return 0;
}

static int x_finalise(nsfb_t *nsfb)
{
    xstate_t *xstate = nsfb->frontend_priv;
    if (xstate == NULL)
        return 0;

    /* free pixmap */
    xcb_free_pixmap(xstate->connection, xstate->pmap);

    /* close connection to server */
    xcb_disconnect(xstate->connection);

    return 0;
}

static bool x_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    xcb_generic_event_t *e;
    xcb_expose_event_t *ee;
    xcb_motion_notify_event_t *emn;
    xcb_button_press_event_t *ebp;
    xstate_t *xstate = nsfb->frontend_priv;

    if (xstate == NULL)
        return false;

    if (timeout < 0) {
        e = xcb_wait_for_event(xstate->connection);
        if (e == NULL) {
            /* I/O error */
            event->type = NSFB_EVENT_CONTROL;
            event->value.controlcode = NSFB_CONTROL_QUIT;
            return true;
        }
    } else {
        e = xcb_poll_for_event(xstate->connection);
        /* Do nothing if there was no event */
        if (e == NULL)
            return false;
    }

    event->type = NSFB_EVENT_NONE;

    switch (e->response_type) {
    case XCB_EXPOSE:
        ee = (xcb_expose_event_t *)e;
        xcb_copy_area(xstate->connection,
                      xstate->pmap,
                      xstate->window,
                      xstate->gc,
                      ee->x, ee->y,
                      ee->x, ee->y,
                      ee->width, ee->height);
        xcb_flush (xstate->connection);
        break;

    case XCB_MOTION_NOTIFY:
        emn = (xcb_motion_notify_event_t *)e;
        event->type = NSFB_EVENT_MOVE_ABSOLUTE;
        event->value.vector.x = emn->event_x;
        event->value.vector.y = emn->event_y;
        event->value.vector.z = 0;
        break;


    case XCB_BUTTON_PRESS:
        ebp = (xcb_button_press_event_t *)e;
        event->type = NSFB_EVENT_KEY_DOWN;

        switch (ebp->detail) {

        case X_BUTTON_LEFT:
            event->value.keycode = NSFB_KEY_MOUSE_1;
            break;

        case X_BUTTON_MIDDLE:
            event->value.keycode = NSFB_KEY_MOUSE_2;
            break;

        case X_BUTTON_RIGHT:
            event->value.keycode = NSFB_KEY_MOUSE_3;
            break;

        case X_BUTTON_WHEELUP:
            event->value.keycode = NSFB_KEY_MOUSE_4;
            break;

        case X_BUTTON_WHEELDOWN:
            event->value.keycode = NSFB_KEY_MOUSE_5;
            break;
        }
        break;

    case XCB_BUTTON_RELEASE:
        ebp = (xcb_button_press_event_t *)e;
        event->type = NSFB_EVENT_KEY_UP;

        switch (ebp->detail) {

        case X_BUTTON_LEFT:
            event->value.keycode = NSFB_KEY_MOUSE_1;
            break;

        case X_BUTTON_MIDDLE:
            event->value.keycode = NSFB_KEY_MOUSE_2;
            break;

        case X_BUTTON_RIGHT:
            event->value.keycode = NSFB_KEY_MOUSE_3;
            break;

        case X_BUTTON_WHEELUP:
            event->value.keycode = NSFB_KEY_MOUSE_4;
            break;

        case X_BUTTON_WHEELDOWN:
            event->value.keycode = NSFB_KEY_MOUSE_5;
            break;
        }
        break;


    }

    /*
      switch (sdlevent.type) {
      case X_KEYDOWN:
      event->type = NSFB_EVENT_KEY_DOWN;
      event->value.keycode = x_nsfb_map[sdlevent.key.keysym.sym];
      break;

      case X_KEYUP:
      event->type = NSFB_EVENT_KEY_UP;
      event->value.keycode = x_nsfb_map[sdlevent.key.keysym.sym];
      break;



      }
    */
    free(e);

    return true;
}

static int x_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    if ((cursor != NULL) &&
        (cursor->plotted == true) &&
        (nsfb_plot_bbox_intersect(box, &cursor->loc))) {
        nsfb_cursor_clear(nsfb, cursor);
    }
    return 0;
}



static int
x_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
    xstate_t *xstate = nsfb->frontend_priv;
    nsfb_bbox_t redraw;
    nsfb_bbox_t fbarea;

    if ((cursor != NULL) && (cursor->plotted == true)) {

        nsfb_plot_add_rect(&cursor->savloc, &cursor->loc, &redraw);

        /* screen area */
        fbarea.x0 = 0;
        fbarea.y0 = 0;
        fbarea.x1 = nsfb->width;
        fbarea.y1 = nsfb->height;

        nsfb_plot_clip(&fbarea, &redraw);

        nsfb_cursor_clear(nsfb, cursor);

        nsfb_cursor_plot(nsfb, cursor);

        /* TODO: This is hediously ineficient - should keep the pointer image
         * as a pixmap and plot server side
         */
        update_and_redraw_pixmap(xstate, redraw.x0, redraw.y0, redraw.x1 - redraw.x0, redraw.y1 - redraw.y0);

    }
    return true;
}


static int x_update(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    xstate_t *xstate = nsfb->frontend_priv;
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    if ((cursor != NULL) &&
	(cursor->plotted == false)) {
        nsfb_cursor_plot(nsfb, cursor);
    }

    update_and_redraw_pixmap(xstate, box->x0, box->y0, box->x1 - box->x0, box->y1 - box->y0);

    xcb_flush(xstate->connection);

    return 0;
}

const nsfb_frontend_rtns_t x_rtns = {
    .initialise = x_initialise,
    .finalise = x_finalise,
    .input = x_input,
    .claim = x_claim,
    .update = x_update,
    .cursor = x_cursor,
    .geometry = x_set_geometry,
};

NSFB_FRONTEND_DEF(x, NSFB_FRONTEND_X, &x_rtns)
