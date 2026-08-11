#include "fishing_bobber.h"
#include "fishing_water.h"
#include "../../config.h"
#include <math.h>
#define BOBBER_GRAVITY   (-14.0f)
#define BOBBER_BOB_AMP    0.06f
#define BOBBER_BOB_RATE   2.2f
#define BOBBER_GRAB_DIST  0.9f
void fishing_bobber_cast(fishing_bobber *b, vec3 origin, vec3 dir, float power) {
    b->pos      = origin;
    b->vel      = vec3_scale(vec3_normalize(dir), power);
    // a bit of upward lob so it arcs instead of firing flat into the water.
    b->vel.y   += power * 0.45f;
    b->landed   = 0;
    b->water    = WATER_NONE;
    b->submerge = 0.0f;
    b->bob_phase = 0.0f;
}

static int cell_is_solid(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return wy < 0;
return block_is_solid(world_get_block(w, wx, wy, wz));
}

// snap the bobber to rest on a found water surface and classify the patch.
static void settle_on_water(fishing_bobber *b, world *w, int wx, int wz) {
    int found = 0;
    int sy = fishing_water_surface_y(w, wx, wz, &found);
    if (!found) { b->water = WATER_NONE; return; }

    // float so the waterline sits across the bobber's middle.
    b->pos.y = (float)sy + 0.85f;
    b->vel   = VEC3_ZERO;
    b->water = fishing_water_classify(w, wx, sy, wz);
    b->landed = 1;
}

void fishing_bobber_update(fishing_bobber *b, world *w, float dt) {
    if (dt <= 0.0f) return;
if (!b->landed) {
        // ballistic step. integrate velocity then position.
        b->vel.y += BOBBER_GRAVITY * dt;
        vec3 next = vec3_add(b->pos, vec3_scale(b->vel, dt));

        int wx = (int)floorf(next.x);
        int wy = (int)floorf(next.y);
        int wz = (int)floorf(next.z);

        if (fishing_water_is_water(w, wx, wy, wz)) {
            settle_on_water(b, w, wx, wz);
            return;
        }
        if (cell_is_solid(w, wx, wy, wz)) {
            // smacked into ground, no fishing here but the line still extends.
            b->pos    = next;
            b->vel    = VEC3_ZERO;
            b->landed = 1;
            b->water  = WATER_NONE;
            return;
        }
        b->pos = next;
        return;
    }

    // landed: only water gets the bob treatment.
    if (b->water == WATER_NONE) return;
b->bob_phase += BOBBER_BOB_RATE * dt;
if (b->bob_phase > 6.2831853f) b->bob_phase -= 6.2831853f;
int wx = (int)floorf(b->pos.x);
int wz = (int)floorf(b->pos.z);
int found = 0;
int sy = fishing_water_surface_y(w, wx, wz, &found);
if (k > 1.0f) k = 1.0f;
b->submerge += (target - b->submerge) * k;
if (b->submerge < 0.0f) b->submerge = 0.0f;
if (b->submerge > 1.0f) b->submerge = 1.0f;
