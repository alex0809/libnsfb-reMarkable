/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported interface for the libnsfb graphics library. 
 */

#ifndef _LIBNSFB_CURSOR_H
#define _LIBNSFB_CURSOR_H 1

/** Initialise the cursor.
 */
bool nsfb_cursor_init(nsfb_t *nsfb);

/** Set cursor parameters.
 *
 * Set a cursor, the cursor will be shown at the specified location and
 * size. The pixel data may be referenced untill the cursor is altered or
 * cleared
 *
 * @param nsfb The frambuffer context.
 * @param loc The location of the cursor
 * @param pixel The pixel data for the cursor 
 */
bool nsfb_cursor_set(nsfb_t *nsfb, const nsfb_colour_t *pixel, int bmp_width, int bmp_height, int bmp_stride);

/** Set cursor location.
 *
 * @param nsfb The frambuffer context.
 * @param loc The location of the cursor
 */
bool nsfb_cursor_loc_set(nsfb_t *nsfb, const nsfb_bbox_t *loc);

/** get the cursor location */
bool nsfb_cursor_loc_get(nsfb_t *nsfb, nsfb_bbox_t *loc);


#endif
