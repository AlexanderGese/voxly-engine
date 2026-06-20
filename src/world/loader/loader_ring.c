#include "loader_ring.h"

#include <stdlib.h>

// wrap a coord into [0, SIDE). C's % goes negative for negatives so we fix that.
static int wrap(int v) {
    int m = v % LOADER_RING_SIDE;
    if (m < 0) m += LOADER_RING_SIDE;
    return m;
}

static int slot_index(int cx, int cz) {
    return wrap(cx) + wrap(cz) * LOADER_RING_SIDE;
}

void loader_ring_init(loader_ring *r) {
    for (int i = 0; i < LOADER_RING_SLOTS; i++) {
        loader_slot *s = &r->slots[i];
        s->cx = s->cz = 0;
        s->stage = LOADER_STAGE_EMPTY;
        s->c = NULL;
        s->gen = 0;
        s->cooldown_us = 0;
        s->occupied = 0;
    }
    r->centre_cx = r->centre_cz = 0;
    r->homed = 0;
}

void loader_ring_free(loader_ring *r, void (*free_chunk)(chunk *c, void *u),
                      void *user) {
    for (int i = 0; i < LOADER_RING_SLOTS; i++) {
        loader_slot *s = &r->slots[i];
        if (s->c) {
            if (free_chunk) free_chunk(s->c, user);
            s->c = NULL;
        }
        s->occupied = 0;
    }
}

int loader_ring_cheby(const loader_ring *r, int cx, int cz) {
    if (!r->homed) return -1;
    int dx = abs(cx - r->centre_cx);
    int dz = abs(cz - r->centre_cz);
    int d = dx > dz ? dx : dz;
    if (d > LOADER_RING_RADIUS) return -1;
    return d;
}

loader_slot *loader_ring_at(loader_ring *r, int cx, int cz) {
    if (loader_ring_cheby(r, cx, cz) < 0) return NULL;
    return &r->slots[slot_index(cx, cz)];
}

loader_slot *loader_ring_slot_for(loader_ring *r, int cx, int cz) {
    loader_slot *s = loader_ring_at(r, cx, cz);
    if (!s) return NULL;
    if (!s->occupied) return NULL;
    if (s->cx != cx || s->cz != cz) return NULL;   // stale, that coord rolled off
    return s;
}

// reset a slot to a fresh EMPTY home at (cx,cz). bumps gen so any job still in the
// queue for the OLD occupant is recognised as stale and dropped on pop.
static void rehome(loader_slot *s, int cx, int cz) {
    s->cx = cx;
    s->cz = cz;
    s->stage = LOADER_STAGE_EMPTY;
    s->c = NULL;          // chunk (if any) was already evicted by the caller
    s->gen++;
    s->cooldown_us = 0;
    s->occupied = 1;
}

int loader_ring_recenter(loader_ring *r, int player_cx, int player_cz,
                         loader_evict_fn evict, void *user) {
    if (r->homed && player_cx == r->centre_cx && player_cz == r->centre_cz)
        return 0;   // didnt cross a chunk border, nothing to do

    int changed = 0;
    int new_lo_x = player_cx - LOADER_RING_RADIUS;
    int new_hi_x = player_cx + LOADER_RING_RADIUS;
    int new_lo_z = player_cz - LOADER_RING_RADIUS;
    int new_hi_z = player_cz + LOADER_RING_RADIUS;

    // walk every coord that's in range of the NEW centre. for each, look at the
    // slot it maps to. if that slot is currently homed on a different coord (the
    // common case after a move), evict the old occupant and rehome. coords that
    // were already correct are left untouched -- that's the whole point of the
    // toroidal layout, only the leading/trailing strips actually move.
    for (int cz = new_lo_z; cz <= new_hi_z; cz++) {
        for (int cx = new_lo_x; cx <= new_hi_x; cx++) {
            loader_slot *s = &r->slots[slot_index(cx, cz)];
            if (s->occupied && s->cx == cx && s->cz == cz)
                continue;   // already ours, keep its progress
            // this slot held some coord that's no longer wanted here. evict it.
            if (s->occupied && s->c && evict)
                evict(s->cx, s->cz, s->c, user);
            rehome(s, cx, cz);
            changed++;
        }
    }

    // slots whose home coord is now out of range still carry stale data; mark them
    // vacated so loader_ring_slot_for rejects them. they'll get rehomed lazily the
    // next time their index is needed, but flagging now keeps queries honest.
    // (we only need to sweep when the jump was large; for a 1-chunk step the loop
    // above already overwrote the relevant strips.)
    int jump_x = abs(player_cx - r->centre_cx);
    int jump_z = abs(player_cz - r->centre_cz);
    if (!r->homed || jump_x > 1 || jump_z > 1) {
        for (int i = 0; i < LOADER_RING_SLOTS; i++) {
            loader_slot *s = &r->slots[i];
            if (!s->occupied) continue;
            int dx = abs(s->cx - player_cx);
            int dz = abs(s->cz - player_cz);
            if (dx > LOADER_RING_RADIUS || dz > LOADER_RING_RADIUS) {
                if (s->c && evict) evict(s->cx, s->cz, s->c, user);
                s->c = NULL;
                s->occupied = 0;
                s->gen++;
            }
        }
    }

    r->centre_cx = player_cx;
    r->centre_cz = player_cz;
    r->homed = 1;
    return changed;
}
