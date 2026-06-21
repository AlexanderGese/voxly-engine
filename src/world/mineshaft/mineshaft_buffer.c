#include "mineshaft_buffer.h"
#include <stdlib.h>

void mineshaft_buffer_init(mineshaft_buffer *b) {
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
}

void mineshaft_buffer_free(mineshaft_buffer *b) {
    free(b->items);
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
}

void mineshaft_buffer_reset(mineshaft_buffer *b) {
    b->count = 0;
}

static int grow(mineshaft_buffer *b) {
    int newcap = b->cap ? b->cap * 2 : 512;
    mineshaft_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
    if (!p) return 0;   // caller checks add() return; we just stop growing
    b->items = p;
    b->cap = newcap;
    return 1;
}

int mineshaft_buffer_add(mineshaft_buffer *b, int x, int y, int z, block_id id) {
    if (b->count >= b->cap && !grow(b)) return 0;
    mineshaft_voxel *v = &b->items[b->count++];
    v->x = x; v->y = y; v->z = z; v->id = id;
    return 1;
}

int mineshaft_buffer_fill_box(mineshaft_buffer *b, mineshaft_box box, block_id id) {
    int n = 0;
    for (int y = box.y0; y < box.y1; y++)
        for (int z = box.z0; z < box.z1; z++)
            for (int x = box.x0; x < box.x1; x++)
                n += mineshaft_buffer_add(b, x, y, z, id);
    return n;
}

int mineshaft_buffer_carve_room(mineshaft_buffer *b, mineshaft_box box,
                                block_id wall, block_id air) {
    int n = 0;
    for (int y = box.y0; y < box.y1; y++) {
        for (int z = box.z0; z < box.z1; z++) {
            for (int x = box.x0; x < box.x1; x++) {
                int on_x = (x == box.x0 || x == box.x1 - 1);
                int on_y = (y == box.y0 || y == box.y1 - 1);
                int on_z = (z == box.z0 || z == box.z1 - 1);
                block_id id = (on_x || on_y || on_z) ? wall : air;
                n += mineshaft_buffer_add(b, x, y, z, id);
            }
        }
    }
    return n;
}

int mineshaft_buffer_frame(mineshaft_buffer *b, mineshaft_box box, int y, block_id id) {
    int n = 0;
    for (int z = box.z0; z < box.z1; z++) {
        for (int x = box.x0; x < box.x1; x++) {
            if (x != box.x0 && x != box.x1 - 1 &&
                z != box.z0 && z != box.z1 - 1) continue;
            n += mineshaft_buffer_add(b, x, y, z, id);
        }
    }
    return n;
}

int mineshaft_buffer_post(mineshaft_buffer *b, int x, int y0, int y1, int z, block_id id) {
    int n = 0;
    for (int y = y0; y < y1; y++)
        n += mineshaft_buffer_add(b, x, y, z, id);
    return n;
}

int mineshaft_buffer_line(mineshaft_buffer *b, int x, int y, int z,
                          int axis, int len, block_id id) {
    int n = 0;
    int step = len >= 0 ? 1 : -1;
    int cnt  = len >= 0 ? len : -len;
    for (int i = 0; i < cnt; i++) {
        int px = x, pz = z;
        if (axis == 0) px += i * step;
        else           pz += i * step;
        n += mineshaft_buffer_add(b, px, y, pz, id);
    }
    return n;
}
