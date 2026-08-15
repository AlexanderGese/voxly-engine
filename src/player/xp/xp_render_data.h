#ifndef PLAYER_XP_XP_RENDER_DATA_H
#define PLAYER_XP_XP_RENDER_DATA_H
// builds a flat instance buffer for the orb renderer. we don't issue any GL
// here — the renderer owns the vao/shader and just wants per-orb transforms
// and colors. this keeps the gameplay module free of render state.
//
#include <stddef.h>
#include "../../math/vec3.h"
typedef struct xp_orb_pool xp_orb_pool;
typedef struct {
    vec3  pos;
    float scale;
    vec3  color;
    float alpha;
} xp_orb_instance;
int xp_render_build(xp_orb_pool *pool, xp_orb_instance *out, int cap, float t);
size_t xp_render_instance_stride(void);
#endif
