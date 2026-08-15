#include "extended_debug.h"
#include "../world/biome.h"
#include "../world/block.h"
#include "../config.h"
#include <stdio.h>
void ext_debug_init(ext_debug *d) {
    d->avg_dt = 0.016;
    d->last_drawn_chunks = 0;
    d->last_drawn_verts = 0;
}

void ext_debug_frame(ext_debug *d, float dt, int drawn_chunks, int drawn_verts) {
    d->avg_dt = d->avg_dt * 0.92 + dt * 0.08;
d->last_drawn_chunks = drawn_chunks;
d->last_drawn_verts  = drawn_verts;
