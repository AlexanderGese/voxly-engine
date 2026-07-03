#ifndef RENDER_BLOOM2_CHAIN_H
#define RENDER_BLOOM2_CHAIN_H

#include "bloom2_config.h"
#include "bloom2_target.h"

// the mip pyramid. chain.mip[0] is half the screen res, each subsequent
// level halves again until we hit BLOOM2_MIN_MIP_DIM or run out of slots.
// downsample writes top->bottom, upsample accumulates bottom->top.

typedef struct {
    bloom2_target mip[BLOOM2_MAX_MIPS];
    int count;          // valid levels (<= requested mip_count)
    int base_w, base_h; // full screen size this chain was built for
} bloom2_chain;

// build a chain for a full-res w x h. requested is the desired mip count;
// the actual count may be smaller if the resolution runs out. returns the
// number of mips created, or 0 on failure.
int  bloom2_chain_build(bloom2_chain *c, int w, int h, int requested);
void bloom2_chain_destroy(bloom2_chain *c);

// figure out how many mips fit a given resolution without allocating. handy
// for the params clamp and for logging.
int  bloom2_chain_fit(int w, int h, int requested);

// dimensions of mip `level`. level 0 is half-res. clamped helpers so callers
// dont have to repeat the >>1 dance.
int  bloom2_chain_mip_w(int base_w, int level);
int  bloom2_chain_mip_h(int base_h, int level);

#endif
