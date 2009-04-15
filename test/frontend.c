#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "libnsfb.h"

int main(int argc, char **argv)
{
    nsfb_t *nsfb;
    const char *fename;
    enum nsfb_frontend_e fetype;

    if (argc < 2) {
        fename="sdl";
    } else {
        fename = argv[1];
    }

    fetype = nsfb_frontend_from_name(fename);
    if (fetype == NSFB_FRONTEND_NONE) {
        fprintf(stderr, "Unable to initialise nsfb with %s frontend\n", fename);
        return 1;
    }

    nsfb = nsfb_init(fetype);
    if (nsfb == NULL) {
        fprintf(stderr, "Unable to initialise nsfb with %s frontend\n", fename);
        return 2;
    }

    if (nsfb_set_geometry(nsfb, 0, 0, 32) == -1) {
        fprintf(stderr, "Unable to set geometry\n");
        nsfb_finalise(nsfb);
        return 3;
    }

    if (nsfb_init_frontend(nsfb) == -1) {
        fprintf(stderr, "Unable to initialise nsfb frontend\n");
        nsfb_finalise(nsfb);
        return 4;
    }

    sleep(2);

    nsfb_finalise(nsfb);
    return 0;
}
