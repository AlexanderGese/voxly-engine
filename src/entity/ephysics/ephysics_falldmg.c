#include "ephysics_falldmg.h"
#include <math.h>

// blocks of fall you get for free. minecraft uses 3; we match it so it feels
// familiar. above that, one point of damage per block.
#define EPHYS_SAFE_FALL_BLOCKS 3.0f

void ephysics_fall_reset(ephys_fall *f) {
    f->fall_start_y = 0.0f;
    f->peak_speed   = 0.0f;
    f->falling      = 0;
}

void ephysics_fall_update(ephys_fall *f, const ephys_body *b) {
    int grounded = (b->flags & EPHYS_F_GROUNDED) != 0;
    int in_water = (b->flags & EPHYS_F_IN_WATER) != 0;

    if (grounded || in_water) {
        // on the ground (or floating): no active fall. damage extraction happens
        // in ephysics_fall_damage on the landing tick before this clears it.
        if (!f->falling) {
            f->fall_start_y = b->pos.y;   // keep the start tracking our feet
        }
        return;
    }

    // airborne. start a fall the first descending tick.
    if (!f->falling && b->vel.y < 0.0f) {
        f->falling      = 1;
        f->fall_start_y = b->pos.y;
        f->peak_speed   = 0.0f;
    }

    if (f->falling) {
        float down = b->vel.y < 0.0f ? -b->vel.y : 0.0f;
        if (down > f->peak_speed) f->peak_speed = down;
        // if we somehow gained height (bounce, updraft) push the start up so we
        // only ever count net descent.
        if (b->pos.y > f->fall_start_y) f->fall_start_y = b->pos.y;
    }
}

float ephysics_fall_distance(const ephys_fall *f, const ephys_body *b) {
    if (!f->falling) return 0.0f;
    float d = f->fall_start_y - b->pos.y;
    return d > 0.0f ? d : 0.0f;
}

int ephysics_fall_damage(ephys_fall *f, const ephys_body *b, int cushioned) {
    int grounded = (b->flags & EPHYS_F_GROUNDED) != 0;
    if (!grounded || !f->falling) return 0;

    float dist = f->fall_start_y - b->pos.y;
    int dmg = 0;

    if (!cushioned && dist > EPHYS_SAFE_FALL_BLOCKS) {
        // one point per block past the freebie. round to nearest so a 4.6 block
        // fall hurts a bit more than a flat truncate would suggest.
        dmg = (int)floorf((dist - EPHYS_SAFE_FALL_BLOCKS) + 0.5f);
        if (dmg < 0) dmg = 0;
    }

    // landed -> fall is over regardless of whether it hurt.
    ephysics_fall_reset(f);
    return dmg;
}
