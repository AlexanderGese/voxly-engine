#include "block_update.h"
#include "block.h"
#include "../config.h"

void buq_init(block_update_queue *q) {
    q->head = 0;
    q->tail = 0;
}

void buq_schedule(block_update_queue *q, int wx, int wy, int wz, int delay) {
    int next = (q->tail + 1) % BLOCK_UPDATE_MAX;
    if (next == q->head) return; // full, drop
    q->queue[q->tail].wx = wx;
    q->queue[q->tail].wy = wy;
    q->queue[q->tail].wz = wz;
    q->queue[q->tail].tick_delay = delay;
    q->tail = next;
}

int buq_pending(const block_update_queue *q) {
    return (q->tail - q->head + BLOCK_UPDATE_MAX) % BLOCK_UPDATE_MAX;
}

static void process_update(world *w, int wx, int wy, int wz) {
    block_id id = world_get_block(w, wx, wy, wz);

    // gravity for sand/gravel
    if (id == BLOCK_SAND) {
        block_id below = world_get_block(w, wx, wy - 1, wz);
        if (below == BLOCK_AIR || below == BLOCK_WATER) {
            world_set_block(w, wx, wy, wz, BLOCK_AIR);
            world_set_block(w, wx, wy - 1, wz, BLOCK_SAND);
        }
    }

    // water flow (simplified)
    if (id == BLOCK_WATER) {
        // try fall
        if (wy > 0 && world_get_block(w, wx, wy - 1, wz) == BLOCK_AIR) {
            world_set_block(w, wx, wy - 1, wz, BLOCK_WATER);
        }
        // try spread sideways
        static const int dx[4] = {1, -1, 0, 0};
        static const int dz[4] = {0, 0, 1, -1};
        for (int d = 0; d < 4; d++) {
            block_id nb = world_get_block(w, wx + dx[d], wy, wz + dz[d]);
            if (nb == BLOCK_AIR) {
                world_set_block(w, wx + dx[d], wy, wz + dz[d], BLOCK_WATER);
            }
        }
    }

    // cactus breaks if neighbor appears
    if (id == BLOCK_CACTUS) {
        static const int dx[4] = {1, -1, 0, 0};
        static const int dz[4] = {0, 0, 1, -1};
        for (int d = 0; d < 4; d++) {
            block_id nb = world_get_block(w, wx + dx[d], wy, wz + dz[d]);
            if (block_is_solid(nb)) {
                world_set_block(w, wx, wy, wz, BLOCK_AIR);
                return;
            }
        }
    }
}

void buq_tick(block_update_queue *q, world *w) {
    int processed = 0;
    while (q->head != q->tail && processed < 64) {
        block_update *u = &q->queue[q->head];
        if (u->tick_delay > 0) {
            u->tick_delay--;
            break; // front of queue still waiting
        }
        process_update(w, u->wx, u->wy, u->wz);
        q->head = (q->head + 1) % BLOCK_UPDATE_MAX;
        processed++;
    }
}
