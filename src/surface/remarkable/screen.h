#include <stdbool.h>

#include "libnsfb.h"

#define TEMP_USE_REMARKABLE_DRAW         0x0018
#define WAVEFORM_MODE_GC16               0x2
#define EPDC_FLAG_USE_REMARKABLE_DITHER  0x300f30

#define DEFAULT_TEMP                     TEMP_USE_REMARKABLE_DRAW
#define DEFAULT_WAVEFORM_MODE            WAVEFORM_MODE_GC16
#define DEFAULT_EPDC_FLAG                EPDC_FLAG_USE_REMARKABLE_DITHER
#define FRAMEBUFFER_FILE                 "/dev/fb0"

struct screen_info {
    int height;
    int width;
    int bpp;
    int linelen;
    int fbsize;
};

int fb;

int load_screen_info(struct screen_info *scrinfo);

int open_fb(void);
int close_fb(void);
int update_region(nsfb_bbox_t *box);
