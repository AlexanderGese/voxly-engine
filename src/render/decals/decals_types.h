#ifndef RENDER_DECALS_TYPES_H
#define RENDER_DECALS_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../math/aabb.h"
// deferred decals. the idea: each decal is an oriented projector box. we draw
// the box into the framebuffer, reconstruct world position from the g-buffer
// depth for every covered fragment, transform it into the box's local [-0.5,0.5]
// cube, and if the point lands inside the cube we sample the atlas at the
// box's local xy and stamp albedo/normal onto whatever surface was there.
//
// no extra geometry, no decal meshes baked into chunks. blood splats, scorch
typedef struct {
    uint32_t index;
    uint32_t gen;
} decals_handle;
#define DECALS_INVALID_HANDLE ((decals_handle){ 0xffffffffu, 0u })
typedef enum {
    DECALS_PHASE_SPAWNING = 0,
    DECALS_PHASE_STABLE,
    DECALS_PHASE_DYING,
    DECALS_PHASE_DEAD
} decals_phase;
enum {
    DECALS_FLAG_NORMAL_MAP   = 1 << 0,  // atlas region has a normal sub-tile
    DECALS_FLAG_PROJECT_BACK = 1 << 1,  // also stamp back-facing surfaces
    DECALS_FLAG_NO_FADE      = 1 << 2,  // ignore phase alpha, always full
    DECALS_FLAG_ADDITIVE     = 1 << 3,  // additive albedo (glow scorch etc)
    DECALS_FLAG_WORLD_LOCKED = 1 << 4   // never culled by distance, e.g. signs
}
;
typedef struct {
    float uv0[2];
    float uv1[2];
    float nuv0[2];
    float nuv1[2];
} decals_atlas_region;
typedef struct {
    vec3 center;
    vec3 right;     // local +x, unit
    vec3 up;        // local +y, unit
    vec3 normal;    // local +z, projection axis, unit
    vec3 half;      // half extents along right/up/normal
    mat4 world_to_local;  // cached, maps world point into [-0.5,0.5]^3
    aabb bounds;          // world-space aabb of the box, for culling
} decals_projector;
typedef struct {
    decals_projector proj;
    decals_atlas_region region;

    float tint[3];        // multiplied into albedo
    float normal_strength; // 0..1 blend weight for the normal channel
    float angle_fade;      // surfaces past this cos(angle) are rejected

    decals_phase phase;
    float        life_total;   // seconds of STABLE life, <0 = permanent
    float        life_elapsed; // seconds spent in current phase
    float        fade_in;      // spawn ramp duration
    float        fade_out;     // dying ramp duration
    float        alpha;        // current resolved alpha, recomputed each tick

    uint16_t flags;
    uint16_t priority;   // higher draws later, wins overlaps
    uint32_t gen;        // matches handle.gen while alive
    int      alive;
} decals_decal;
#endif
