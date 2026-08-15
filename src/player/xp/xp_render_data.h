#ifndef PLAYER_XP_XP_RENDER_DATA_H
#define PLAYER_XP_XP_RENDER_DATA_H

// builds a flat instance buffer for the orb renderer. we don't issue any GL
// here — the renderer owns the vao/shader and just wants per-orb transforms
// and colors. this keeps the gameplay module free of render state.
//
// one instance is { vec3 pos, float scale, vec3 color, float alpha }, laid
// out tightly so it can be memcpy'd straight into a vbo if the caller likes.

#include <stddef.h>

#include "../../math/vec3.h"

typedef struct xp_orb_pool xp_orb_pool;

typedef struct {
    vec3  pos;
    float scale;
    vec3  color;
    float alpha;
} xp_orb_instance;

// fill `out` (capacity `cap`) with one instance per live orb. returns the
// number written (<= cap; extra orbs are dropped, which only matters if the
// renderer's buffer is somehow smaller than the pool). `t` is a global time
// in seconds used for the cosmetic bob/spin.
int xp_render_build(xp_orb_pool *pool, xp_orb_instance *out, int cap, float t);

// number of bytes one instance occupies, for vbo sizing. just sizeof, but
// exposed so the renderer doesn't include the struct layout assumptions.
size_t xp_render_instance_stride(void);

#endif
