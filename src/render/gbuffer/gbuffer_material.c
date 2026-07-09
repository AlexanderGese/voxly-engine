#include "gbuffer_material.h"

#include <math.h>

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static uint8_t to_u8(float v) {
    int x = (int)lrintf(clamp01(v) * 255.0f);
    if (x < 0)   x = 0;
    if (x > 255) x = 255;
    return (uint8_t)x;
}

// per-block roughness/metallic table. only the interesting ones differ from
// the default chalky dielectric. keep this aligned with the block enum.
static gbuffer_material default_for(block_id id) {
    gbuffer_material m = { 0.85f, 0.0f, 0.0f, GBUF_MAT_FLAG_NONE };
    switch (id) {
        case BLOCK_STONE:  m.roughness = 0.7f;  break;
        case BLOCK_COBBLE: m.roughness = 0.9f;  break;
        case BLOCK_SAND:   m.roughness = 0.95f; break;
        case BLOCK_GLASS:  m.roughness = 0.05f; break;
        case BLOCK_ICE:    m.roughness = 0.12f; break;
        case BLOCK_LEAVES: m.roughness = 0.8f;  m.flags |= GBUF_MAT_FLAG_FOLIAGE; break;
        case BLOCK_GRASS:  m.roughness = 0.78f; break;
        case BLOCK_WATER:  m.roughness = 0.04f; m.flags |= GBUF_MAT_FLAG_WATER; break;
        case BLOCK_SNOW:   m.roughness = 0.6f;  break;
        case BLOCK_BRICK:  m.roughness = 0.88f; break;
        case BLOCK_TORCH:  m.roughness = 0.9f;  break;
        default: break;
    }
    return m;
}

gbuffer_material gbuffer_material_from_block(block_id id) {
    gbuffer_material m = default_for(id);

    const block_info *bi = block_get(id);
    if (bi && bi->emits_light) {
        // luminance is 0..15, normalise and bias up so torches read bright
        m.emissive = clamp01((float)bi->luminance / (float)MAX_LIGHT);
        m.emissive = m.emissive * 0.7f + 0.3f;
        m.flags |= GBUF_MAT_FLAG_EMISSIVE;
    }
    return m;
}

gbuffer_mat_texel gbuffer_material_pack(gbuffer_material m) {
    gbuffer_mat_texel t;
    t.r = to_u8(m.roughness);
    t.g = to_u8(m.metallic);
    t.b = to_u8(m.emissive);
    // flags live in the high nibble, low nibble reserved (used to stash a
    // coarse emissive overflow bit once, removed it, kept the room)
    t.a = (uint8_t)((m.flags & 0x0f) << 4);
    return t;
}

gbuffer_material gbuffer_material_unpack(gbuffer_mat_texel t) {
    gbuffer_material m;
    m.roughness = (float)t.r / 255.0f;
    m.metallic  = (float)t.g / 255.0f;
    m.emissive  = (float)t.b / 255.0f;
    m.flags     = (uint8_t)((t.a >> 4) & 0x0f);
    return m;
}

gbuffer_material gbuffer_material_lerp(gbuffer_material a, gbuffer_material b, float t) {
    t = clamp01(t);
    gbuffer_material r;
    r.roughness = a.roughness + (b.roughness - a.roughness) * t;
    r.metallic  = a.metallic  + (b.metallic  - a.metallic ) * t;
    r.emissive  = a.emissive  + (b.emissive  - a.emissive ) * t;
    r.flags     = (uint8_t)(a.flags | b.flags);
    return r;
}
