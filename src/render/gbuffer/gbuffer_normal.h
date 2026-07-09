#ifndef RENDER_GBUFFER_NORMAL_H
#define RENDER_GBUFFER_NORMAL_H

#include <stdint.h>
#include "../../math/vec3.h"

// octahedral normal encoding. the normal attachment is RGB10A2 so we have
// ~10 bits per channel to spend, but we only encode into two of them with
// the octahedral mapping which is plenty for world normals and leaves the
// third channel for a tiny bit of extra data (face id, used by debug view).
//
// classic mapping, see meyer et al. "on floating-point normal vectors".

typedef struct { float u, v; } gbuffer_oct;

// fold a unit vector onto the octahedron and project to the [-1,1] square.
gbuffer_oct gbuffer_normal_encode(vec3 n);

// inverse of the above. result is normalized.
vec3 gbuffer_normal_decode(gbuffer_oct o);

// pack/unpack to the 10:10 fixed point the texture stores. the 2-bit alpha
// carries `face` (0..3) which we map back to a cardinal axis for debugging.
uint32_t gbuffer_normal_pack(vec3 n, int face);
vec3     gbuffer_normal_unpack(uint32_t packed, int *face_out);

#endif
