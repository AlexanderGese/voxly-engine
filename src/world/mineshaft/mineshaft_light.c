#include "mineshaft_light.h"
#include "mineshaft_rand.h"

// candidate torch slots: the four wall midpoints of a cell, set one block below
// the ceiling so the torch reads as wall-mounted rather than floating.
typedef struct { int x, y, z; } torch_slot;

static int gather_slots(mineshaft_box box, int ceil_y, torch_slot out[4]) {
    int mx = (box.x0 + box.x1) / 2;
    int mz = (box.z0 + box.z1) / 2;
    int ty = ceil_y - 1;
    out[0] = (torch_slot){ box.x0,     ty, mz };       // -x wall
    out[1] = (torch_slot){ box.x1 - 1, ty, mz };       // +x wall
    out[2] = (torch_slot){ mx,         ty, box.z0 };    // -z wall
    out[3] = (torch_slot){ mx,         ty, box.z1 - 1 };// +z wall
    return 4;
}

float mineshaft_light_survival(const mineshaft_grid *g, int cx, int cz) {
    if (!mineshaft_grid_in_bounds(g, cx, cz)) return 0.0f;
    uint8_t kind = g->cells[cz * g->w + cx].kind;
    switch (kind) {
        case MS_CELL_JUNCTION: return 0.6f;   // crossings stayed lit longest
        case MS_CELL_SHAFT:    return 0.5f;   // you light a ladder landing
        case MS_CELL_ROOM:     return 0.45f;  // work rooms had lamps
        case MS_CELL_CORRIDOR: return 0.18f;  // most corridor torches died
        default:               return 0.05f;  // dead ends are pitch black
    }
}

int mineshaft_light_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                         mineshaft_box cell_box, int floor_y, int ceil_y,
                         float survival, uint32_t seed) {
    (void)floor_y;
    if (survival <= 0.0f) return 0;

    torch_slot slots[4];
    int ns = gather_slots(cell_box, ceil_y, slots);

    int n = 0;
    for (int i = 0; i < ns; i++) {
        const torch_slot *s = &slots[i];
        // stable per-slot roll so the same torches survive across regen.
        float r = mineshaft_hash_f01(s->x, s->y, s->z, seed ^ 0x7041c400u);
        if (r < survival)
            n += mineshaft_buffer_add(b, s->x, s->y, s->z, cfg->mat_torch);
    }
    return n;
}
