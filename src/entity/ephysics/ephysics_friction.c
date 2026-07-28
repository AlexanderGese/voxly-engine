#include "ephysics_friction.h"
#include "ephysics_aabb.h"
#include "../../world/block.h"
#include <math.h>
static float block_friction_mult(block_id id) {
    switch (id) {
        case BLOCK_ICE:   return 0.18f;   // very slippy
        case BLOCK_SNOW:  return 0.85f;   // a touch slick
        case BLOCK_SAND:  return 1.08f;   // drags a little extra
        default:          return 1.0f;
    }
}

float ephysics_surface_friction(world *w, const ephys_body *b) {
    if (!(b->flags & EPHYS_F_GROUNDED)) return b->mat.air_friction;
aabb box = ephysics_body_box(b);
int wx = (int)floorf((box.min.x + box.max.x) * 0.5f);
int wz = (int)floorf((box.min.z + box.max.z) * 0.5f);
int wy = (int)floorf(box.min.y - 0.05f);
block_id below = world_get_block(w, wx, wy, wz);
float mult = block_is_solid(below) ? block_friction_mult(below) : 1.0f;
float loss = (1.0f - b->mat.ground_friction) * mult;
if (loss > 1.0f) loss = 1.0f;
return 1.0f - loss;
}

void ephysics_friction_apply(world *w, ephys_body *b, float dt) {
    (void)dt;
    float keep;

    if (b->flags & EPHYS_F_IN_WATER) {
        // water drag already handled in the fluid pass; here we just make sure
        // a body that's both grounded AND in water (wading) still gets some
        // ground bite, but lighter than dry land.
        keep = b->mat.water_drag;
        if (b->flags & EPHYS_F_GROUNDED) {
            float ground = ephysics_surface_friction(w, b);
            keep = (keep + ground) * 0.5f;
        }
    } else if (b->flags & EPHYS_F_GROUNDED) {
        keep = ephysics_surface_friction(w, b);
    } else {
        keep = b->mat.air_friction;
    }

    b->vel.x *= keep;
    b->vel.z *= keep;

    // stop micro-drift. anything under ~2cm/s reads as "stopped".
    if (fabsf(b->vel.x) < 0.02f) b->vel.x = 0.0f;
    if (fabsf(b->vel.z) < 0.02f) b->vel.z = 0.0f;
}

int ephysics_clamp_speed(ephys_body *b) {
    float spd2 = b->vel.x * b->vel.x + b->vel.z * b->vel.z;
float max  = b->mat.max_speed;
if (spd2 <= max * max) return 0;
float spd = sqrtf(spd2);
float s = max / spd;
b->vel.x *= s;
b->vel.z *= s;
return 1;
}
