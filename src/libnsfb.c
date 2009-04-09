#include <stdio.h>
#include <malloc.h>

#include "libnsfb.h"
#include "nsfb.h"
#include "frontend.h"


/* documented in libnsfb.h */
nsfb_t*
nsfb_init(const enum nsfb_frontend_e frontend_type)
{
    nsfb_t *newfb;
    newfb = calloc(1, sizeof(nsfb_t));
    if (newfb == NULL)
        return NULL;

    /* obtain frontend routines */
    newfb->frontend_rtns = nsfb_frontend_get_rtns(frontend_type);
    if (newfb->frontend_rtns == NULL) {
        free(newfb);
        return NULL;
    }

    newfb->frontend_rtns->defaults(newfb);

    return newfb;
}

int nsfb_finalise(nsfb_t *nsfb)
{
    return nsfb->frontend_rtns->finalise(nsfb);
}


int
nsfb_init_frontend(nsfb_t *nsfb)
{
    return nsfb->frontend_rtns->initialise(nsfb);
}

int nsfb_input(nsfb_t *nsfb)
{
    return nsfb->frontend_rtns->input(nsfb);
}

int nsfb_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    return nsfb->frontend_rtns->claim(nsfb, box);
}

int nsfb_release(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    return nsfb->frontend_rtns->release(nsfb, box);
}

int nsfb_get_geometry(nsfb_t *nsfb, int *width, int *height, int *bpp) 
{
    if (width != NULL)
        *width = nsfb->width;

    if (height != NULL)
        *height = nsfb->height;

    if (bpp != NULL)
        *bpp = nsfb->bpp;

    return 0;
}

int nsfb_get_framebuffer(nsfb_t *nsfb, uint8_t **ptr, int *linelen) 
{
    *ptr = nsfb->ptr;
    *linelen = nsfb->linelen;
    return 0;
}
