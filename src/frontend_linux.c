#include "libnsfb.h"
#include "nsfb.h"
#include "frontend.h"

const nsfb_frontend_rtns_t linux_rtns = {
    .foo = 2,
};

NSFB_FRONTEND_DEF(linux, NSFB_FRONTEND_LINUX, &linux_rtns)
