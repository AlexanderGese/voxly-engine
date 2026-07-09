#ifndef RENDER_GBUFFER_MATERIAL_H
#define RENDER_GBUFFER_MATERIAL_H

#include <stdint.h>
#include "../../math/vec3.h"
#include "../../world/block.h"
#include "../../config.h"   // MAX_LIGHT

// surface material as the cpu sees it before we pack it into the g-buffer's
// material attachment. roughness/metallic are the usual pbr knobs, emissive
// is 0..1 (driven by block luminance), and flags carries a few bits the
// shading pass cares about.

enum {
    GBUF_MAT_FLAG_NONE    = 0,
    GBUF_MAT_FLAG_EMISSIVE= 1 << 0,  // self-lit, skip directional shading
    GBUF_MAT_FLAG_FOLIAGE = 1 << 1,  // double-sided, wind-tinted
    GBUF_MAT_FLAG_WATER   = 1 << 2,  // animated normal, fresnel rim
    GBUF_MAT_FLAG_METAL   = 1 << 3,  // hint, lets us bias metallic up
};

typedef struct {
    float roughness;   // 0 = mirror, 1 = chalk
    float metallic;    // 0 = dielectric, 1 = conductor
    float emissive;    // 0..1 self emission scalar
    uint8_t flags;     // GBUF_MAT_FLAG_*
} gbuffer_material;

// the four 8-bit channels of the packed material texel, in order r,g,b,a.
typedef struct {
    uint8_t r, g, b, a;
} gbuffer_mat_texel;

// derive a material straight from a block id. uses block_info luminance to
// fill emissive and a small lookup for roughness/metallic per known block.
gbuffer_material gbuffer_material_from_block(block_id id);

// pack/unpack the cpu material into the 4-byte texel the shader reads.
// flags get quantized into the top nibble of the alpha channel.
gbuffer_mat_texel gbuffer_material_pack(gbuffer_material m);
gbuffer_material  gbuffer_material_unpack(gbuffer_mat_texel t);

// blend two materials (used when a face straddles two block types, eg snow
// over dirt). t=0 -> a, t=1 -> b. flags are or'd, not lerped.
gbuffer_material gbuffer_material_lerp(gbuffer_material a, gbuffer_material b, float t);

#endif
