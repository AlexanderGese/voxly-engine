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
mineshaft_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
if (!p) return 0;
b->items = p;
b->cap = newcap;
return 1;
for (int y = box.y0;
y < box.y1;
y++)
        for (int z = box.z0;
z < box.z1;
z++)
            for (int x = box.x0;
x < box.x1;
x++)
                n += mineshaft_buffer_add(b, x, y, z, id);
return n;
for (int z = box.z0;
z < box.z1;
int step = len >= 0 ? 1 : -1;
int cnt  = len >= 0 ? len : -len;
for (int i = 0;
i < cnt;
}
