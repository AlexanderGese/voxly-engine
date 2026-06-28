#include "structgen_buffer.h"
#include <stdlib.h>

void structgen_buffer_init(structgen_buffer *b) {
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
}

void structgen_buffer_free(structgen_buffer *b) {
    free(b->items);
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
}

void structgen_buffer_reset(structgen_buffer *b) {
    b->count = 0;
}

static int grow(structgen_buffer *b) {
    int newcap = b->cap ? b->cap * 2 : 256;
    structgen_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
    if (!p) return 0;   // caller checks add() return; we just stop growing
    b->items = p;
    b->cap = newcap;
    return 1;
}

int structgen_buffer_add(structgen_buffer *b, int x, int y, int z, block_id id) {
    if (b->count >= b->cap && !grow(b)) return 0;
    structgen_voxel *v = &b->items[b->count++];
    v->x = x; v->y = y; v->z = z; v->id = id;
    return 1;
}

int structgen_buffer_fill_box(structgen_buffer *b, structgen_box box, block_id id) {
    int n = 0;
    for (int y = box.y0; y < box.y1; y++)
        for (int z = box.z0; z < box.z1; z++)
            for (int x = box.x0; x < box.x1; x++)
                n += structgen_buffer_add(b, x, y, z, id);
    return n;
}

int structgen_buffer_fill_shell(structgen_buffer *b, structgen_box box, block_id id) {
    int n = 0;
    for (int y = box.y0; y < box.y1; y++) {
        for (int z = box.z0; z < box.z1; z++) {
            for (int x = box.x0; x < box.x1; x++) {
                int on_x = (x == box.x0 || x == box.x1 - 1);
                int on_y = (y == box.y0 || y == box.y1 - 1);
                int on_z = (z == box.z0 || z == box.z1 - 1);
                if (!(on_x || on_y || on_z)) continue;   // interior, skip
                n += structgen_buffer_add(b, x, y, z, id);
            }
        }
    }
    return n;
}

int structgen_buffer_fill_frame(structgen_buffer *b, structgen_box box, int y, block_id id) {
    int n = 0;
    for (int z = box.z0; z < box.z1; z++) {
        for (int x = box.x0; x < box.x1; x++) {
            if (x != box.x0 && x != box.x1 - 1 &&
                z != box.z0 && z != box.z1 - 1) continue;
            n += structgen_buffer_add(b, x, y, z, id);
        }
    }
    return n;
}
