/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported interface for the libnsfb graphics library. 
 */

#ifndef _LIBNSFB_H
#define _LIBNSFB_H 1

#include <stdint.h>

typedef struct nsfb_cursor_s nsfb_cursor_t;
typedef struct nsfb_s nsfb_t;
typedef struct nsfb_event_s nsfb_event_t;

/** co-ordinate for plotting operations */
typedef struct nsfb_point_s {
    int x;
    int y;
} nsfb_point_t;

/** bounding box for plotting operations */
typedef struct nsfb_bbox_s {
    int x0;
    int y0;
    int x1;
    int y1;
} nsfb_bbox_t;

/** The type of frontend for a framebuffer context. */
enum nsfb_frontend_e {
    NSFB_FRONTEND_NONE = 0, /**< Empty frontend.  */
    NSFB_FRONTEND_SDL, /**< SDL frontend */
    NSFB_FRONTEND_LINUX, /**< Linux frontend */
    NSFB_FRONTEND_VNC, /**< VNC frontend */
    NSFB_FRONTEND_ABLE, /**< ABLE frontend */
    NSFB_FRONTEND_RAM /**< RAM frontend */
};

/** Initialise nsfb context.
 *
 * This initialises a framebuffer context.
 *
 * @param frontend The type of frontend to create a context for.
 */
nsfb_t *nsfb_init(enum nsfb_frontend_e frontend);

/** Finalise nsfb.
 *
 * This shuts down and releases all resources associated with an nsfb context.
 *
 * @param nsfb The context returned from ::nsfb_init tofinalise
 */
int nsfb_finalise(nsfb_t *nsfb);

/** Initialise selected frontend.
 *
 * @param nsfb The context returned from ::nsfb_init
 */
int nsfb_init_frontend(nsfb_t *nsfb);

/** Select frontend type from a name.
 * 
 * @param name The name to select a frontend.
 * @return The frontend type or NSFB_FRONTEND_NONE if frontend with specified 
 *         name was not available
 */
enum nsfb_frontend_e nsfb_frontend_from_name(const char *name);

/** Claim an area of screen to be redrawn.
 *
 * Informs the nsfb library that an area of screen will be directly
 * updated by the user program. This is neccisarry so the library can
 * ensure the soft cursor plotting is correctly handled. After the
 * update has been perfomed ::nsfb_update should be called.
 *
 * @param box The bounding box of the area which might be altered.
 */
int nsfb_claim(nsfb_t *nsfb, nsfb_bbox_t *box);

/** Update an area of screen which has been redrawn.
 *
 * Informs the nsfb library that an area of screen has been directly
 * updated by the user program. Some frontends only show the update on
 * notification. The area updated does not neccisarrily have to
 * corelate with a previous ::nsfb_claim bounding box, however if the
 * redrawn area is larger than the claimed area pointer plotting
 * artifacts may occour.
 *
 * @param box The bounding box of the area which has been altered.
 */
int nsfb_update(nsfb_t *nsfb, nsfb_bbox_t *box);

/** Obtain the geometry of a nsfb context.
 *
 * @param width a variable to store the framebuffer width in or NULL
 * @param height a variable to store the framebuffer height in or NULL
 * @param bpp a variable to store the framebuffer bpp in or NULL
 */
int nsfb_get_geometry(nsfb_t *nsfb, int *width, int *height, int *bpp);

/** Alter the geometry of a framebuffer context
 *
 * @param nsfb The context to alter.
 * @param width The new display width.
 * @param height The new display height.
 * @param bpp The new display depth. 
 */
int nsfb_set_geometry(nsfb_t *nsfb, int width, int height, int bpp);

/** Obtain the framebuffer memory base and stride. 
 */
int nsfb_get_framebuffer(nsfb_t *nsfb, uint8_t **ptr, int *linelen);

#endif
