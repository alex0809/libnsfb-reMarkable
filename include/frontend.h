#include "libnsfb.h" /* exported interface */
#include "nsfb.h"

/* frontend default options */
typedef int (nsfb_fendfn_defaults_t)(nsfb_t *nsfb);
/* frontend init */
typedef int (nsfb_fendfn_init_t)(nsfb_t *nsfb);
/* frontend finalise */
typedef int (nsfb_fendfn_fini_t)(nsfb_t *nsfb);
/* frontend input */
typedef int (nsfb_fendfn_input_t)(nsfb_t *nsfb);
/* frontend area claim */
typedef int (nsfb_fendfn_claim_t)(nsfb_t *nsfb, nsfb_bbox_t *box);
/* frontend area release */
typedef int (nsfb_fendfn_release_t)(nsfb_t *nsfb, nsfb_bbox_t *box);

typedef struct nsfb_frontend_rtns_s {
    nsfb_fendfn_defaults_t *defaults;
    nsfb_fendfn_init_t *initialise;
    nsfb_fendfn_fini_t *finalise;
    nsfb_fendfn_input_t *input;
    nsfb_fendfn_claim_t *claim;
    nsfb_fendfn_release_t *release;
} nsfb_frontend_rtns_t;

void _nsfb_register_frontend(const enum nsfb_frontend_e type, const nsfb_frontend_rtns_t *rtns, const char *name);

nsfb_frontend_rtns_t *nsfb_frontend_get_rtns(enum nsfb_frontend_e type);

/* macro which adds a builtin command with no argument limits */
#define NSFB_FRONTEND_DEF(__name, __type, __rtns)                       \
    static void __name##_register_frontend(void) __attribute__((constructor)); \
    void __name##_register_frontend(void) {                              \
        _nsfb_register_frontend(__type, __rtns, #__name);               \
    }                                                                   
