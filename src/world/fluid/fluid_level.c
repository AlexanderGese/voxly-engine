#include "fluid_level.h"

// max visual height of a full fluid block. slightly less than 1 so the
// surface sits a touch below the block top like in mc.
#define VOXL_FLUID_TOP_BIAS  0.875f

int voxl_fluid_level_clamp(int level) {
    if (level < VOXL_FLUID_EMPTY) return VOXL_FLUID_EMPTY;
    if (level > VOXL_FLUID_FULL)  return VOXL_FLUID_FULL;
    return level;
}

int voxl_fluid_level_add(voxl_fluid_cell *c, uint8_t kind, int amount) {
    if (!c || amount <= 0) return amount > 0 ? amount : 0;
    // taking on a new fluid into an empty cell sets its kind
    if (voxl_fluid_cell_empty(c)) {
        c->kind = kind;
        c->level = 0;
    } else if (c->kind != kind) {
        // cant mix two different fluids, reject the whole amount
        return amount;
    }
    int total = c->level + amount;
    if (total > VOXL_FLUID_FULL) {
        int overflow = total - VOXL_FLUID_FULL;
        c->level = VOXL_FLUID_FULL;
        return overflow;
    }
    c->level = (uint8_t)total;
    return 0;
}

int voxl_fluid_level_remove(voxl_fluid_cell *c, int amount) {
    if (!c || amount <= 0) return 0;
    int took = amount;
    if (took > c->level) took = c->level;
    c->level = (uint8_t)(c->level - took);
    if (c->level == 0) {
        c->kind = VOXL_FLUID_AIR;
        c->falling = 0;
    }
    return took;
}

float voxl_fluid_level_fraction(const voxl_fluid_cell *c) {
    if (!c || voxl_fluid_cell_empty(c)) return 0.0f;
    if (c->falling) return 1.0f;
    return (float)c->level / (float)VOXL_FLUID_FULL;
}

float voxl_fluid_surface_height(const voxl_fluid_cell *c, int y) {
    float frac = voxl_fluid_level_fraction(c);
    if (frac <= 0.0f) return (float)y;
    return (float)y + frac * VOXL_FLUID_TOP_BIAS;
}

float voxl_fluid_corner_height(float a, float b, float c, float d) {
    // ignore any zero (no-fluid) samples so a corner next to land doesnt
    // get dragged down to the floor.
    float sum = 0.0f;
    int n = 0;
    if (a > 0.0f) { sum += a; n++; }
    if (b > 0.0f) { sum += b; n++; }
    if (c > 0.0f) { sum += c; n++; }
    if (d > 0.0f) { sum += d; n++; }
    if (n == 0) return 0.0f;
    return sum / (float)n;
}
