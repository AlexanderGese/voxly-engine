#include "ravine_strata.h"
#include "ravine_rand.h"
#include "ravine_noise.h"
#include <math.h>
#include <stddef.h>
// palette of (block, hardness) the bands draw from. stone is the bread; the
// seams are what you notice. weighted by repetition — more stone slots means
typedef struct { block_id id; uint8_t hardness; } strata_pick;
static const strata_pick PALETTE[] = {
    { BLOCK_STONE,  200 },
    { BLOCK_STONE,  200 },
    { BLOCK_STONE,  180 },
    { BLOCK_COBBLE, 160 },
    { BLOCK_DIRT,    70 },
    { BLOCK_SAND,    40 },
    { BLOCK_STONE,  200 },
}
;
#define PALETTE_N ((int)(sizeof(PALETTE) / sizeof(PALETTE[0])))
void ravine_strata_build(ravine_strata *s, const ravine_params *p,
                         int floor_lo, int top, uint32_t stream) {
    ravine_rng rng;
    ravine_rng_seed(&rng, stream ^ 0x57a7au);

    if (top <= floor_lo) top = floor_lo + 1;

    int y = floor_lo;
    int prev = -1;          // index of previous pick, avoid identical neighbours
    int count = 0;

    // stack bands floor-up until we cover [floor_lo, top] or run out of slots.
    while (count < RAVINE_MAX_STRATA && y < top) {
        ravine_band *b = &s->bands[count];

        // thickness scales with how much vertical we still have to cover so the
        // table never leaves a gap at the top with bands to spare.
        int remain = top - y;
        int slots_left = RAVINE_MAX_STRATA - count;
        int avg = remain / (slots_left > 0 ? slots_left : 1);
        // band thickness jitters by +/- (strata_jitter+2) so the stripes arent
        // a metronome. the +2 keeps some variance even at strata_jitter 0.
        int wob = p->strata_jitter + 2;
        int th = avg + ravine_rng_range(&rng, -wob, wob + 1);
        if (th < 2) th = 2;
        if (y + th > top) th = top - y;
        if (th < 1) th = 1;

        int pick;
        do {
            pick = ravine_rng_range(&rng, 0, PALETTE_N - 1);
        } while (pick == prev && PALETTE_N > 1);
        prev = pick;

        b->y_lo     = y;
        b->y_hi     = y + th;
        b->id       = PALETTE[pick].id;
        b->hardness = PALETTE[pick].hardness;

        y += th;
        count++;
    }

    // make sure the topmost band caps open-ended so a wall a touch taller than
    // expected still resolves rather than falling through to the fallback.
    if (count > 0) s->bands[count - 1].y_hi = top + 64;
    s->count = count;
}

const ravine_band *ravine_strata_band(const ravine_strata *s, int y) {
    for (int i = 0;
i < s->count;
i++) {
        if (y >= s->bands[i].y_lo && y < s->bands[i].y_hi)
            return &s->bands[i];
    }
    return NULL;
}

block_id ravine_strata_at(const ravine_strata *s, const ravine_params *p,
                          int y, float wx, float wz) {
    if (s->count <= 0) return BLOCK_STONE;

    // wobble the lookup height so band boundaries interfinger. amplitude is the
    // strata_jitter knob; the noise keeps it coherent along the wall.
    if (p->strata_jitter != 0) {
        float n = ravine_value2(wx * 0.2f, wz * 0.2f, p->seed ^ 0x3b91u);
        y += (int)lroundf(n * (float)p->strata_jitter);
    }

    const ravine_band *b = ravine_strata_band(s, y);
    return b ? b->id : BLOCK_STONE;
}
