#ifndef ENTITY_SPAWN_MSPAWN_DESPAWN_H
#define ENTITY_SPAWN_MSPAWN_DESPAWN_H
#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "../mob.h"
// despawn policy. two bands, lifted straight from how the classic does it:
// past the hard radius a mob is gone immediately, between the soft and hard
// radii it rolls a slow per-tick chance so the world thins out gradually as
// the player walks away instead of mobs popping out behind a fence. passives
// that the player has interacted with would be exempt, but we dont track that
// yet so everything is fair game.
#define MSPAWN_DESPAWN_HARD   80.0f   // beyond this, vanish now
#define MSPAWN_DESPAWN_SOFT   48.0f   // beyond this, start rolling to leave
#define MSPAWN_DESPAWN_CHANCE  0.02f  // per-tick leave chance in the soft band
// per-mob despawn timer state, parallel to the registry by index. the soft
// band needs a little memory so a mob that lingers at the edge eventually goes
// rather than re-rolling fresh forever.
typedef struct {
    float linger[MAX_MOBS];   // seconds spent in the soft band
} mspawn_despawn_state;
void mspawn_despawn_init(mspawn_despawn_state *st);
int  mspawn_despawn_tick(mspawn_despawn_state *st, mob_registry *mr,
                         vec3 player_pos, mspawn_rng *r, float dt);
#endif
