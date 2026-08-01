#ifndef ENTITY_SPAWN_MSPAWN_GROUND_H
#define ENTITY_SPAWN_MSPAWN_GROUND_H

#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "../../world/world.h"

// turning a rough world-xz into a standable site. the driver throws darts in a
// ring around the player; this module catches each one, walks down to find a
// surface, and decides whether a mob of the requested kind could actually
// stand there (headroom, light band, ground type, y band). pure reads off the
// world, never mutates it.

// how tall a column of clear air we insist on above the floor. zombies are
// ~1.8 tall so two blocks is the floor; spiders are squat but i dont special
// case it, two blocks of air never hurt anyone.
#define MSPAWN_HEADROOM 2

// scan the column at (wx, wz) for the topmost solid surface at or below y_hint,
// returning the floor y (the solid block's y) via *out_y, or -1 if none in
// range. y_hint lets the driver bias toward the surface vs deep caves.
int mspawn_find_floor(world *w, int wx, int wz, int y_hint, int *out_y);

// combined light (block vs sun, max of the two) one block above the floor,
// which is where the mob's feet sit. 0..15.
int mspawn_light_at(world *w, int wx, int wy_floor, int wz);

// does the entry's contract hold at this floor? checks ground kind, headroom,
// y band, and the light window. does NOT check density/caps; thats the
// driver's job. returns 1 and fills *out on success.
int mspawn_site_ok(world *w, const mspawn_entry *e, int wx, int wy_floor,
                   int wz, uint32_t seed, mspawn_site *out);

// one-shot: roll a candidate around (cx,cz) within radius, find a floor, and
// validate it for the entry. returns 1 with a ready site, else 0. this is the
// workhorse the driver and the pack builder both lean on.
int mspawn_try_site(world *w, const mspawn_entry *e, int cx, int cz,
                    int radius, int y_hint, mspawn_rng *r, mspawn_site *out);

#endif
