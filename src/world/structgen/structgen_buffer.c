#include "structgen_buffer.h"
#include <stdlib.h>
b->items = NULL;
b->count = 0;
b->cap = 0;
structgen_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
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
