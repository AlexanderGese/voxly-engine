#include "bloom2_chain.h"
#include "../../util/log.h"

#include <stddef.h>

int bloom2_chain_mip_w(int base_w, int level) {
    // level 0 = half res, so shift by level+1.
    int w = base_w >> (level + 1);
    return w < 1 ? 1 : w;
}

int bloom2_chain_mip_h(int base_h, int level) {
    int h = base_h >> (level + 1);
    return h < 1 ? 1 : h;
}

int bloom2_chain_fit(int w, int h, int requested) {
    if (requested > BLOOM2_MAX_MIPS) requested = BLOOM2_MAX_MIPS;
    if (requested < 1) requested = 1;

    int n = 0;
    for (int i = 0; i < requested; i++) {
        int mw = bloom2_chain_mip_w(w, i);
        int mh = bloom2_chain_mip_h(h, i);
        if (mw < BLOOM2_MIN_MIP_DIM || mh < BLOOM2_MIN_MIP_DIM)
            break;
        n++;
    }
    // always give back at least one usable mip, even on a tiny window.
    if (n < 1) n = 1;
    return n;
}

int bloom2_chain_build(bloom2_chain *c, int w, int h, int requested) {
    for (int i = 0; i < BLOOM2_MAX_MIPS; i++) {
        c->mip[i].fbo = 0;
        c->mip[i].tex = 0;
        c->mip[i].w = c->mip[i].h = 0;
    }
    c->base_w = w;
    c->base_h = h;
    c->count  = 0;

    int want = bloom2_chain_fit(w, h, requested);

    for (int i = 0; i < want; i++) {
        int mw = bloom2_chain_mip_w(w, i);
        int mh = bloom2_chain_mip_h(h, i);
        if (!bloom2_target_create(&c->mip[i], mw, mh)) {
            LOGE("bloom2: failed building mip %d (%dx%d)", i, mw, mh);
            bloom2_chain_destroy(c);
            return 0;
        }
        c->count++;
    }

    LOGI("bloom2: chain built, %d mips for %dx%d (top mip %dx%d)",
         c->count, w, h, c->mip[0].w, c->mip[0].h);
    return c->count;
}

void bloom2_chain_destroy(bloom2_chain *c) {
    for (int i = 0; i < BLOOM2_MAX_MIPS; i++)
        bloom2_target_destroy(&c->mip[i]);
    c->count = 0;
    c->base_w = c->base_h = 0;
}
