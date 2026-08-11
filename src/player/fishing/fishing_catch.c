#include "fishing_catch.h"
#include <stddef.h>

int fishing_catch_weight(const fishing_catch *c, fishing_rng *r) {
    if (!c) return 1;

    int base;
    switch (c->category) {
    case CATCH_TREASURE: base = 7; break;   // shiny and stubborn
    case CATCH_FISH:     base = 4; break;
    case CATCH_JUNK:     base = 2; break;   // limp, comes in easy
    default:             base = 3; break;
    }

    // a couple points of variance so no two of the same catch fight identically.
    int jitter = r ? fishing_rng_range(r, -1, 2) : 0;
    int w = base + jitter;
    if (w < 1)  w = 1;
    if (w > 10) w = 10;
    return w;
}

const char *fishing_catch_name(const fishing_catch *c) {
    if (!c) return "nothing";
    switch (c->category) {
    case CATCH_FISH:     return "fish";
    case CATCH_TREASURE: return "treasure";
    case CATCH_JUNK:     return "junk";
    default:             return "nothing";
    }
}

int fishing_catch_is_valid(const fishing_catch *c) {
    return c && c->category != CATCH_NONE && c->count > 0 && c->block != BLOCK_AIR;
}
