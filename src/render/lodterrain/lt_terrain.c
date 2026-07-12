#include "lt_terrain.h"
#include "lt_build.h"
#include "lt_mesh.h"
#include "lt_seam.h"
#include "lt_neighbor.h"
#include "../../math/aabb.h"
#include "../../config.h"
#include <stddef.h>
if (wy < 0) return 0;
if (wy >= CHUNK_SIZE_Y) return MAX_LIGHT;
int bl = world_get_blocklight(w, wx, wy, wz);
int sl = world_get_sunlight(w, wx, wy, wz);
return bl > sl ? bl : sl;
lt_select_default(&t->select);
t->build_budget = build_budget > 0 ? build_budget : 4;
t->enabled = 1;
lt_mesh_init(&t->scratch);
}

// per-chunk visit context. world_visit hands us one chunk at a time.
typedef struct {
    lt_terrain *t;
    world      *w;
    vec3        cam;
    float       dt;
} update_ctx;
