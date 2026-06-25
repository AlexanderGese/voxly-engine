#include "stronghold_door.h"

// we dont have a real door block in the palette, so plank doors are faked with
// a planks block in the lower hole + air above (you "open" by mining, classic
// early-build behavior) and iron bars are faked with glass: see-through, blocks
// movement. grates are a half-height gap with a planks lintel.

stronghold_door_kind stronghold_door_classify(const stronghold_room *r,
                                              const stronghold_door *d,
                                              stronghold_rng *rng) {
    // gated flag from the corridor pass wins outright: bars.
    if (d->gated) return STRONGHOLD_DOOR_BARS;

    switch (r->type) {
        case STRONGHOLD_ROOM_PORTAL:
            return STRONGHOLD_DOOR_BARS;          // sealed approach
        case STRONGHOLD_ROOM_LIBRARY:
            return STRONGHOLD_DOOR_PLANK;         // libraries get proper doors
        case STRONGHOLD_ROOM_PRISON:
            return STRONGHOLD_DOOR_BARS;
        case STRONGHOLD_ROOM_STAIRWELL:
            return STRONGHOLD_DOOR_GRATE;         // low headroom into a shaft
        default:
            // mostly open arches, sometimes a stray plank door for variety.
            return stronghold_rng_chance(rng, 0.25f)
                 ? STRONGHOLD_DOOR_PLANK : STRONGHOLD_DOOR_ARCH;
    }
}

int stronghold_door_decorate(const stronghold_room *r, const stronghold_door *d,
                             stronghold_door_kind kind, stronghold_buffer *out) {
    (void)r;
    int x = d->x, y = d->y, z = d->z;
    int n = 0;
    switch (kind) {
        case STRONGHOLD_DOOR_ARCH:
            // already air from the carve. drop a brick lintel one above for the
            // arched look (carve left a solid lintel, so this is a no-op mostly,
            // but stamp it anyway in case a room edit cleared it).
            n += stronghold_buffer_add(out, x, y + 2, z, BLOCK_BRICK);
            break;
        case STRONGHOLD_DOOR_PLANK:
            // planks fill the lower cell, air above. reads as a closed door.
            n += stronghold_buffer_add(out, x, y,     z, BLOCK_PLANKS);
            n += stronghold_buffer_add(out, x, y + 1, z, BLOCK_AIR);
            break;
        case STRONGHOLD_DOOR_BARS:
            // glass stand-in for iron bars: see-through, full block. two high.
            n += stronghold_buffer_add(out, x, y,     z, BLOCK_GLASS);
            n += stronghold_buffer_add(out, x, y + 1, z, BLOCK_GLASS);
            break;
        case STRONGHOLD_DOOR_GRATE:
            // crawl gap: bottom open, planks lintel at head height.
            n += stronghold_buffer_add(out, x, y,     z, BLOCK_AIR);
            n += stronghold_buffer_add(out, x, y + 1, z, BLOCK_PLANKS);
            break;
    }
    return n;
}

int stronghold_door_decorate_all(const stronghold_graph *g, stronghold_buffer *out,
                                 stronghold_rng *rng) {
    int n = 0;
    for (int i = 0; i < g->room_count; i++) {
        const stronghold_room *r = &g->rooms[i];
        for (int dn = 0; dn < r->door_count; dn++) {
            stronghold_door_kind k = stronghold_door_classify(r, &r->doors[dn], rng);
            n += stronghold_door_decorate(r, &r->doors[dn], k, out);
        }
    }
    return n;
}
