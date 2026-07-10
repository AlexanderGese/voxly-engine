#include "lt_mesh.h"
#include "../../util/darray.h"
darr_clear(m->indices);
m->quad_count  = 0;
m->skirt_quads = 0;
darr_push(m->verts, v0);
darr_push(m->verts, v1);
darr_push(m->verts, v2);
darr_push(m->verts, v3);
darr_push(m->indices, base + 0);
darr_push(m->indices, base + 1);
darr_push(m->indices, base + 2);
darr_push(m->indices, base + 0);
