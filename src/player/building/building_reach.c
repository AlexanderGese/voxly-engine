#include "building_reach.h"
#include "building_target.h"

#include "../../config.h"
#include <math.h>

vec3 building_eye_from_feet(vec3 feet) {
    return vec3_new(feet.x, feet.y + PLAYER_EYE_HEIGHT, feet.z);
}

vec3 building_forward_from_angles(float yaw, float pitch) {
    // yaw=0 looks down -z (north), pitch>0 looks up. standard fps basis.
    float cp = cosf(pitch);
    vec3 f = vec3_new(
        -sinf(yaw) * cp,
         sinf(pitch),
        -cosf(yaw) * cp
    );
    return vec3_normalize(f);
}

vec3 building_cell_nearest(vec3 eye, int x, int y, int z) {
    // clamp the eye into the unit cube [x,x+1]^3. clamped point is the nearest
    // point on (or in) the cell.
    float nx = eye.x;
    float ny = eye.y;
    float nz = eye.z;

    if (nx < (float)x)        nx = (float)x;
    if (nx > (float)x + 1.0f) nx = (float)x + 1.0f;
    if (ny < (float)y)        ny = (float)y;
    if (ny > (float)y + 1.0f) ny = (float)y + 1.0f;
    if (nz < (float)z)        nz = (float)z;
    if (nz > (float)z + 1.0f) nz = (float)z + 1.0f;

    return vec3_new(nx, ny, nz);
}

float building_reach_to_hit(const building_target *t, vec3 eye) {
    if (!t || !t->valid) return 1e30f;
    vec3 near = building_cell_nearest(eye, t->hit_x, t->hit_y, t->hit_z);
    return vec3_distance(eye, near);
}

int building_in_reach(const building_target *t, vec3 eye, float reach) {
    if (!t || !t->valid) return 0;
    return building_reach_to_hit(t, eye) <= reach;
}

building_target building_reach_resolve(world *w, vec3 eye, vec3 forward,
                                       float reach) {
    // cast a touch beyond the limit so a block whose center is just past reach
    // but whose near face is inside still registers; then clamp by surface.
    building_target t = building_target_resolve(w, eye, forward, reach + 1.0f);
    if (!t.valid) return t;

    if (!building_in_reach(&t, eye, reach)) {
        building_target miss = {0};
        return miss;
    }
    return t;
}
