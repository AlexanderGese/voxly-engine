#ifndef WORLD_LOADER_RING_H
#define WORLD_LOADER_RING_H
#include <stdint.h>
#include "loader_types.h"
#include "../chunk.h"
// the resident set. a square ring of chunk slots centred on the player, stored
// as a flat toroidal grid so re-centring when the player walks is O(moved) not
// O(all). this is the classic "the player is always at the middle of a wrapping
// array" trick -- index by coord mod side, and a slot is valid only if its stored
// coord matches what that index SHOULD hold for the current centre.
// side length of the grid. LOAD_DISTANCE chunks each way plus the centre, plus a
// 1-chunk skirt so the mesher always has its neighbours resident.
#define LOADER_RING_SKIRT  1
#define LOADER_RING_RADIUS (LOAD_DISTANCE + LOADER_RING_SKIRT)
#define LOADER_RING_SIDE   (2 * LOADER_RING_RADIUS + 1)
#define LOADER_RING_SLOTS  (LOADER_RING_SIDE * LOADER_RING_SIDE)
// one slot in the ring. `gen` is the ABA serial handed to jobs; it bumps every
// time the slot is re-homed to a new coord so stale jobs get dropped.
typedef struct {
    int          cx, cz;       // coord this slot currently represents
    loader_stage stage;
    chunk       *c;            // owned chunk, NULL until ALLOC runs
    uint32_t     gen;          // serial, see loader_job
    uint64_t     cooldown_us;  // dont re-queue before this time (after a FAIL)
    int          occupied;     // 0 = this slot's coord isnt in range right now
} loader_slot;
typedef struct {
    loader_slot slots[LOADER_RING_SLOTS];
    int centre_cx, centre_cz;  // player chunk the ring is currently homed on
    int homed;                 // 0 before the first re-centre
} loader_ring;
void loader_ring_init(loader_ring *r);
// free every owned chunk. caller is responsible for telling the world first if
// the chunk was handed off. used on teardown.
void loader_ring_free(loader_ring *r, void (*free_chunk)(chunk *c, void *u),
                      void *user);
// the slot a coord maps to, or NULL if that coord is outside the current ring.
// O(1). may return a slot whose stored coord != (cx,cz) if it's stale -- callers
// that care use loader_ring_slot_for which validates.
loader_slot *loader_ring_at(loader_ring *r, int cx, int cz);
// like _at but returns NULL unless the slot actually represents (cx,cz) AND is
// occupied. this is the "is this chunk resident" query.
loader_slot *loader_ring_slot_for(loader_ring *r, int cx, int cz);
// re-home the ring on a new player chunk. slots that fall out of range are marked
// vacated (their chunk is handed to `evict`), slots newly in range are reset to
// EMPTY. returns the number of slots that changed coord.
typedef void (*loader_evict_fn)(int cx, int cz, chunk *c, void *user);
int loader_ring_recenter(loader_ring *r, int player_cx, int player_cz,
                         loader_evict_fn evict, void *user);
// chebyshev distance from the ring centre. -1 if outside.
int loader_ring_cheby(const loader_ring *r, int cx, int cz);
#endif
