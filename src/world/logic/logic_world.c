#include "logic_world.h"
#include "logic_block.h"
#include "logic_signal.h"
#include <stddef.h>

// the world's block grid doesn't store per-block facing (block_id is a single
// byte), so a scanned logic block adopts a default facing. interactive
// placement goes through logic_net_place which can set a real facing.
#define LOGIC_DEFAULT_FACING LOGIC_DIR_PX

int logic_world_scan_chunk(logic_net *net, const chunk *c) {
    if (!c) return 0;
    int found = 0;
    int base_x = c->cx * CHUNK_SIZE_X;
    int base_z = c->cz * CHUNK_SIZE_Z;

    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                block_id id = chunk_get_block(c, x, y, z);
                if (!logic_block_is_logic(id)) continue;

                int wx = base_x + x;
                int wy = y;
                int wz = base_z + z;

                logic_cell *cell = logic_grid_put(&net->grid, wx, wy, wz, id);
                if (!cell) continue;
                cell->kind = id;
                if (logic_block_is_directional(id) && cell->facing >= LOGIC_DIR_COUNT)
                    cell->facing = LOGIC_DEFAULT_FACING;
                // schedule it so its initial state settles on the next tick.
                logic_net_wake(net, wx, wy, wz, 0);
                found++;
            }
        }
    }

    if (found) net->dirty_all = 1;
    return found;
}

int logic_world_scan_all(logic_net *net, world *w) {
    if (!w) return 0;
    int total = 0;
    // walk the chunk list. world exposes a visitor but it wants a callback with
    // a single user pointer, so we bundle net+counter.
    for (world_node *n = w->head; n; n = n->next) {
        total += logic_world_scan_chunk(net, n->c);
    }
    return total;
}

void logic_world_on_block_changed(logic_net *net, world *w,
                                  int wx, int wy, int wz) {
    block_id id = world_get_block(w, wx, wy, wz);

    if (logic_block_is_logic(id)) {
        logic_cell *cell = logic_grid_get(&net->grid, wx, wy, wz);
        if (cell && cell->kind != id) {
            // block changed type in place: refresh kind, reset volatile state.
            cell->kind = id;
            cell->power = 0;
            cell->next = 0;
            cell->phase = 0;
            cell->flags &= ~(LOGIC_CF_STAGED | LOGIC_CF_LATCHED);
        } else if (!cell) {
            logic_net_place(net, wx, wy, wz, id, LOGIC_DEFAULT_FACING);
            return;
        }
        net->dirty_all = 1;
        logic_net_wake(net, wx, wy, wz, 0);
    } else {
        // the block is no longer a logic block (broken, replaced by stone).
        if (logic_grid_get(&net->grid, wx, wy, wz)) {
            logic_net_remove(net, wx, wy, wz);
        } else {
            // a plain block changed next to logic - still wake the area so a
            // torch's support change or a new conductor is noticed.
            logic_net_wake(net, wx, wy, wz, 1);
        }
    }
}

int logic_world_lamp_lit(const logic_net *net, int wx, int wy, int wz) {
    logic_cell *c = logic_grid_get((logic_grid *)&net->grid, wx, wy, wz);
    if (!c || c->kind != LOGIC_BLOCK_LAMP) return 0;
    // a lamp lights when any neighbour feeds it any power at all.
    for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
        int nx = wx, ny = wy, nz = wz;
        switch (d) {
            case LOGIC_DIR_PX: nx++; break;
            case LOGIC_DIR_NX: nx--; break;
            case LOGIC_DIR_PY: ny++; break;
            case LOGIC_DIR_NY: ny--; break;
            case LOGIC_DIR_PZ: nz++; break;
            case LOGIC_DIR_NZ: nz--; break;
        }
        logic_cell *n = logic_grid_get((logic_grid *)&net->grid, nx, ny, nz);
        if (n && n->power > 0) return 1;
    }
    return 0;
}

uint8_t logic_world_light_at(const logic_net *net, int wx, int wy, int wz) {
    logic_cell *c = logic_grid_get((logic_grid *)&net->grid, wx, wy, wz);
    if (!c) return 0;
    if (c->kind == LOGIC_BLOCK_LAMP)
        return logic_world_lamp_lit(net, wx, wy, wz) ? LOGIC_MAX : 0;
    if (c->kind == LOGIC_BLOCK_TORCH)
        return c->power > 0 ? (uint8_t)(LOGIC_MAX / 2) : 0;
    return 0;
}
