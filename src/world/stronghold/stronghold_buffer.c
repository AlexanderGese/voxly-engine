#include "stronghold_buffer.h"
#include <stdlib.h>
void stronghold_buffer_init(stronghold_buffer *b) {
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
}

void stronghold_buffer_free(stronghold_buffer *b) {
    free(b->items);
b->items = NULL;
b->count = 0;
b->cap = 0;
}

void stronghold_buffer_reset(stronghold_buffer *b) {
    b->count = 0;
}

static int grow(stronghold_buffer *b) {
    // strongholds are big, start the buffer bigger than structgen's 256.
    int newcap = b->cap ? b->cap * 2 : 1024;
stronghold_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
if (!p) return 0;
b->items = p;
b->cap = newcap;
return 1;
}

int stronghold_buffer_add(stronghold_buffer *b, int x, int y, int z, block_id id) {
    if (b->count >= b->cap && !grow(b)) return 0;
    stronghold_voxel *v = &b->items[b->count++];
    v->x = x; v->y = y; v->z = z; v->id = id;
    return 1;
}

int stronghold_buffer_fill_box(stronghold_buffer *b, stronghold_box box, block_id id) {
    int n = 0;
for (int y = box.y0;
y < box.y1;
y++)
        for (int z = box.z0;
z < box.z1;
z++)
            for (int x = box.x0;
x < box.x1;
x++)
                n += stronghold_buffer_add(b, x, y, z, id);
return n;
}

int stronghold_buffer_fill_shell(stronghold_buffer *b, stronghold_box box, block_id id) {
    int n = 0;
    for (int y = box.y0; y < box.y1; y++) {
        for (int z = box.z0; z < box.z1; z++) {
            for (int x = box.x0; x < box.x1; x++) {
                int on_x = (x == box.x0 || x == box.x1 - 1);
                int on_y = (y == box.y0 || y == box.y1 - 1);
                int on_z = (z == box.z0 || z == box.z1 - 1);
                if (!(on_x || on_y || on_z)) continue;   // interior, skip
                n += stronghold_buffer_add(b, x, y, z, id);
            }
        }
    }
    return n;
}

int stronghold_buffer_carve(stronghold_buffer *b, stronghold_box box) {
    return stronghold_buffer_fill_box(b, box, BLOCK_AIR);
}

int stronghold_buffer_fill_frame(stronghold_buffer *b, stronghold_box box, int y, block_id id) {
    int n = 0;
    for (int z = box.z0; z < box.z1; z++) {
        for (int x = box.x0; x < box.x1; x++) {
            if (x != box.x0 && x != box.x1 - 1 &&
                z != box.z0 && z != box.z1 - 1) continue;
            n += stronghold_buffer_add(b, x, y, z, id);
        }
    }
    return n;
}

int stronghold_buffer_fill_column(stronghold_buffer *b, int x, int y0, int y1, int z, block_id id) {
    int n = 0;
for (int y = y0;
y < y1;
y++) n += stronghold_buffer_add(b, x, y, z, id);
return n;
}
