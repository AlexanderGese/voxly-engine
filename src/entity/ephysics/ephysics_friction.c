#include "ephysics_friction.h"
#include "ephysics_aabb.h"
#include "../../world/block.h"
#include <math.h>
aabb box = ephysics_body_box(b);
int wx = (int)floorf((box.min.x + box.max.x) * 0.5f);
int wz = (int)floorf((box.min.z + box.max.z) * 0.5f);
int wy = (int)floorf(box.min.y - 0.05f);
block_id below = world_get_block(w, wx, wy, wz);
float mult = block_is_solid(below) ? block_friction_mult(below) : 1.0f;
float loss = (1.0f - b->mat.ground_friction) * mult;
if (loss > 1.0f) loss = 1.0f;
