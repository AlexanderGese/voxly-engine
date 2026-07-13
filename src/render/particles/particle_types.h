#ifndef RENDER_PARTICLES_PARTICLE_TYPES_H
#define RENDER_PARTICLES_PARTICLE_TYPES_H

// core particle data. one hot struct, kept small so a full pool fits in
// a couple of cache lines worth of pages. no virtuals, no per-particle
// callbacks — behaviour lives in the emitter desc, not the particle.

#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include <stdint.h>

// hard ceiling on a single system's pool. if you need more than this you
// probably want a second system anyway. picked to keep the sort cheap.
#define PARTICLES_MAX_POOL   8192

// a particle billboard is two triangles = 6 verts. used when sizing the
// gpu scratch buffer.
#define PARTICLES_VERTS_PER  6

typedef enum {
    // how the quad orients itself each frame.
    PARTICLES_BILLBOARD_SPHERICAL = 0,  // always faces camera fully
    PARTICLES_BILLBOARD_CYLINDRICAL,    // locked to world up (smoke, fire)
    PARTICLES_BILLBOARD_VELOCITY,       // stretched along travel (sparks)
    PARTICLES_BILLBOARD_FLAT            // world-axis quad, no facing
} particles_billboard_kind;

typedef enum {
    PARTICLES_BLEND_ALPHA = 0,   // standard src-alpha / one-minus
    PARTICLES_BLEND_ADD          // additive, for glows and embers
} particles_blend_mode;

// the per-particle record. age/life drive every over-lifetime curve so we
// only store the raw simulation state here and resolve color/size later.
typedef struct {
    vec3   pos;
    vec3   vel;
    vec4   color;     // current resolved rgba, written by the sim each step
    float  size;      // birth size (half-extent), fixed at spawn
    float  render_size; // size * size_curve(t), what the billboard uses
    float  age;       // seconds since spawn
    float  life;      // total lifetime in seconds (0 -> dead/free)
    float  rot;       // billboard roll, radians
    float  rot_vel;   // roll speed
    float  seed;      // 0..1 stable per-particle randomness for curves
    uint16_t emitter; // which emitter spawned it (for batching/debug)
    uint16_t flags;
} particles_particle;

#define PARTICLES_FLAG_ALIVE   (1u << 0)
#define PARTICLES_FLAG_SORTED  (1u << 1)
#define PARTICLES_FLAG_GROUND  (1u << 2)  // touched ground, collision latch

// the vertex we feed the gpu. matches the engine's interleaved layout idea:
// position, uv, then a packed color. one of these per corner.
typedef struct {
    float x, y, z;
    float u, v;
    float r, g, b, a;
} particles_vertex;

static inline int particles_is_alive(const particles_particle *p) {
    return (p->flags & PARTICLES_FLAG_ALIVE) && p->life > 0.0f;
}

#endif
