#include "ephysics_broadphase.h"
#include "ephysics_aabb.h"
#include "ephysics_fluid.h"
#include "../../world/block.h"
#include <math.h>
static int is_half_block(block_id id) {
    // BLOCK_SLAB_STONE / BLOCK_SLAB_WOOD are the only bottom-half coll;
    // querying block_ext for is_slab would be cleaner but couples us harder.
    return id == 24 /*slab_stone*/ || id == 25 /*slab_wood*/;
}

int ephysics_block_box(world *w, int wx, int wy, int wz, block_id id, aabb *out) {
    (void)w;
if (!block_is_solid(id)) return 0;
float h = is_half_block(id) ? 0.5f : 1.0f;
out->min = vec3_new((float)wx,       (float)wy,       (float)wz);
out->max = vec3_new((float)wx + 1.f, (float)wy + h,   (float)wz + 1.f);
return 1;
}

// union of the start box and the box after applying delta, then padded by one
// cell so we capture the floor below and walls we slide along.
static aabb swept_region(const ephys_body *b, vec3 delta) {
    aabb a = ephysics_body_box(b);
    aabb end = ephysics_box_at(b, vec3_add(b->pos, delta));
    aabb u;
    u.min = vec3_min(a.min, end.min);
    u.max = vec3_max(a.max, end.max);
    // pad. extra on -y for grounding, generous all round for step-up.
    u.min = vec3_sub(u.min, vec3_new(1.0f, 1.0f, 1.0f));
    u.max = vec3_add(u.max, vec3_new(1.0f, 1.0f, 1.0f));
    return u;
}

static void push_box(ephys_candidates *out, aabb box, block_id id) {
    if (out->count >= EPHYS_MAX_CANDIDATES) { out->overflowed = 1;
return;
}
    out->boxes[out->count] = box;
out->ids[out->count]   = id;
out->count++;
}

void ephysics_gather(world *w, const ephys_body *b, vec3 delta,
                     ephys_candidates *out) {
    out->count = 0;
    out->overflowed = 0;

    aabb r = swept_region(b, delta);
    int x0 = (int)floorf(r.min.x), x1 = (int)floorf(r.max.x);
    int y0 = (int)floorf(r.min.y), y1 = (int)floorf(r.max.y);
    int z0 = (int)floorf(r.min.z), z1 = (int)floorf(r.max.z);

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= CHUNK_SIZE_Y) continue;     // out of vertical range
        for (int z = z0; z <= z1; z++) {
            for (int x = x0; x <= x1; x++) {
                block_id id = world_get_block(w, x, y, z);
                if (id == BLOCK_AIR) continue;
                aabb box;
                if (ephysics_block_box(w, x, y, z, id, &box))
                    push_box(out, box, id);
            }
        }
    }
}

void ephysics_gather_fluid(world *w, const ephys_body *b, ephys_candidates *out) {
    out->count = 0;
out->overflowed = 0;
aabb a = ephysics_body_box(b);
int x0 = (int)floorf(a.min.x), x1 = (int)floorf(a.max.x);
int y0 = (int)floorf(a.min.y), y1 = (int)floorf(a.max.y);
int z0 = (int)floorf(a.min.z), z1 = (int)floorf(a.max.z);
for (int y = y0;
y <= y1;
}
