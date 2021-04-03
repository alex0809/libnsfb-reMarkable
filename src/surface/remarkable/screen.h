#include <stdbool.h>

#include "libnsfb.h"
#include "mxcfb.h"

#define TEMP_USE_REMARKABLE_DRAW         0x0018

#define WAVEFORM_MODE_GC16               0x2
#define WAVEFORM_MODE_GC4	             0x3
#define FAST_WAVEFORM_MODE               WAVEFORM_MODE_GC4
#define SLOW_WAVEFORM_MODE               WAVEFORM_MODE_GC16

#define EPDC_FLAG_USE_REMARKABLE_DITHER  0x300f30

#define DEFAULT_TEMP                     TEMP_USE_REMARKABLE_DRAW
#define DEFAULT_WAVEFORM_MODE            FAST_WAVEFORM_MODE
#define DEFAULT_EPDC_FLAG                EPDC_FLAG_USE_REMARKABLE_DITHER

#define FRAMEBUFFER_FILE                 "/dev/fb0"

typedef struct fb_info_s {
    int height;
    int width;
    int bpp;
    int linelen;
    int fbsize;
} fb_info_t;

typedef struct fb_state_s {
    int fb;
    int fb_size;
    fb_info_t scrinfo;
    void *mapped_fb;
} fb_state_t;

int fb_update_region(fb_state_t *fb_state, nsfb_bbox_t *box);
int fb_initialize(fb_state_t *fb_state);
int fb_finalize(fb_state_t *fb_state);
