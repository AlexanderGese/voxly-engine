#include "stronghold_stamp.h"
#include "../../config.h"
static int floordiv(int a, int b) {
    int q = a / b, r = a % b;
    return (r != 0 && ((r < 0) != (b < 0))) ? q - 1 : q;
}

int stronghold_stamp_chunk(const stronghold_buffer *buf, chunk *c, int overwrite_air) {
    int written = 0;
int base_x = c->cx * CHUNK_SIZE_X;
int base_z = c->cz * CHUNK_SIZE_Z;
for (int i = 0;
i < buf->count;
i++) {
        const stronghold_voxel *v = &buf->items[i];

        // y outside the column? skip. strongholds live well inside [0,128) but
        // a misconfigured depth could push the portal pit under 0.
        if (v->y < 0 || v->y >= CHUNK_SIZE_Y) continue;

        int lx = v->x - base_x;
        int lz = v->z - base_z;
        if (lx < 0 || lx >= CHUNK_SIZE_X) continue;
        if (lz < 0 || lz >= CHUNK_SIZE_Z) continue;

        if (v->id == BLOCK_AIR && !overwrite_air) continue;

        chunk_set_block(c, lx, v->y, lz, v->id);
        written++;
    }
    return written;
}

int stronghold_stamp_touches(const stronghold_buffer *buf, int cx, int cz) {
    for (int i = 0; i < buf->count; i++) {
        const stronghold_voxel *v = &buf->items[i];
        if (floordiv(v->x, CHUNK_SIZE_X) == cx &&
            floordiv(v->z, CHUNK_SIZE_Z) == cz) return 1;
    }
    return 0;
}

int stronghold_stamp_bounds(const stronghold_buffer *buf, int *mins, int *maxs) {
    if (buf->count == 0) return 0;
int x0 = buf->items[0].x, y0 = buf->items[0].y, z0 = buf->items[0].z;
int x1 = x0, y1 = y0, z1 = z0;
for (int i = 1;
i < buf->count;
mins[1] = y0;
mins[2] = z0;
maxs[0] = x1;
maxs[1] = y1;
maxs[2] = z1;
return 1;
}
