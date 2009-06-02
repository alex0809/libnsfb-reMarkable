/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "frontend.h"
#include "plotters.h"

#define MAX_FRONTENDS 16

struct nsfb_frontend_s {
    enum nsfb_frontend_e type;
    const nsfb_frontend_rtns_t *rtns;
    const char *name;
};

static struct nsfb_frontend_s frontends[MAX_FRONTENDS];
static int frontend_count = 0;

/* internal routine which lets frontends register their presence at runtime */
void _nsfb_register_frontend(const enum nsfb_frontend_e type, 
                             const nsfb_frontend_rtns_t *rtns, 
                             const char *name)
{
    if (frontend_count >= MAX_FRONTENDS)
        return; /* no space for additional frontends */

    frontends[frontend_count].type = type;
    frontends[frontend_count].rtns = rtns;
    frontends[frontend_count].name = name;
    frontend_count++;
}

/* default frontend implementations */
static int frontend_defaults(nsfb_t *nsfb)
{
    nsfb->width = 800;
    nsfb->height = 600;
    nsfb->bpp = 16;

    /* select default sw plotters for bpp */
    select_plotters(nsfb);

    return 0;
}

static int frontend_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    nsfb=nsfb;
    box=box;
    return 0;
}

static int frontend_release(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    nsfb=nsfb;
    box=box;
    return 0;
}

static int frontend_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
    nsfb=nsfb;
    cursor=cursor;
    return 0;
}

nsfb_frontend_rtns_t *nsfb_frontend_get_rtns(enum nsfb_frontend_e type)
{
    int fend_loop;
    nsfb_frontend_rtns_t *rtns = NULL;

    for (fend_loop = 0; fend_loop < frontend_count; fend_loop++) {
        if (frontends[fend_loop].type == type) {
            rtns = malloc(sizeof(nsfb_frontend_rtns_t));
            memcpy(rtns, 
                   frontends[fend_loop].rtns, 
                   sizeof(nsfb_frontend_rtns_t));

            /* frontend must have an initialisor, finaliser and input method */
            if ((rtns->initialise == NULL) || 
                (rtns->finalise == NULL) || 
                (rtns->input == NULL) ) {
                free(rtns);
                rtns = NULL;
            }

            /* The rest may be empty but to avoid the null check every time
             * provide default implementations. 
             */
            if (rtns->defaults == NULL) 
                rtns->defaults = frontend_defaults;

            if (rtns->claim == NULL) 
                rtns->claim = frontend_claim;

            if (rtns->release == NULL) 
                rtns->release = frontend_release;

            if (rtns->cursor == NULL) 
                rtns->cursor = frontend_cursor;

            break;
        }
    }
    return rtns;
}

enum nsfb_frontend_e nsfb_frontend_from_name(const char *name)
{
    int fend_loop;

    for (fend_loop = 0; fend_loop < frontend_count; fend_loop++) {
        if (strcmp(frontends[fend_loop].name, name) == 0)
            return frontends[fend_loop].type;
    }
    return NSFB_FRONTEND_NONE;
}
