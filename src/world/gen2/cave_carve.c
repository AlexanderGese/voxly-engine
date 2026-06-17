#include "cave_carve.h"
#include "gen2_noise.h"

#include <math.h>

// we dont have engine 3d noise handy that wont clobber global state, so we
// fake 3d by combining a few 2d value-noise slices at offset planes. cheap
// but gives believable blobs and tunnels for this experiment.

static float pseudo3d(float x, float y, float z, uint32_t seed) {
    // blend three orthogonal 2d slices. each plane uses a distinct seed.
    float a = voxl_gen2_value2(x, z, seed);
    float b = voxl_gen2_value2(x, y, seed + 1337u);
    float c = voxl_gen2_value2(y, z, seed + 7331u);
    return (a + b + c) / 3.0f;   // ~[-1,1]
}

float gen2_cave_density(int wx, int wy, int wz, uint32_t seed) {
    float s = 1.0f / 24.0f;
    float n = pseudo3d((float)wx * s, (float)wy * s, (float)wz * s, seed ^ 0xCA7Eu);
    // add a finer octave for detail
    float d = pseudo3d((float)wx * s * 2.3f, (float)wy * s * 2.3f,
                       (float)wz * s * 2.3f, seed ^ 0x0DD1u);
    float v = 0.7f * n + 0.3f * d;
    return 0.5f * (v + 1.0f);   // -> [0,1]
}

int gen2_cave_is_tunnel(int wx, int wy, int wz, uint32_t seed) {
    // two ridged fields; where both are high we get a thin overlapping
    // channel == a tunnel. squeezing y harder makes them mostly horizontal.
    float s = 1.0f / 40.0f;
    float r1 = voxl_gen2_ridge2((float)wx * s, (float)wz * s, seed ^ 0x7011u, 2);
    float r2 = voxl_gen2_ridge2((float)wx * s + (float)wy * 0.05f,
                                (float)wz * s - (float)wy * 0.05f, seed ^ 0x7022u, 2);
    return (r1 > 0.78f && r2 > 0.7f);
}

int gen2_cave_is_carved(int wx, int wy, int wz, int surface_y, uint32_t seed) {
    if (wy < 2) return 0;                 // keep bedrock-ish floor
    if (wy > surface_y - 4) return 0;     // solid crust under the surface

    if (gen2_cave_density(wx, wy, wz, seed) > 0.72f) return 1;
    if (gen2_cave_is_tunnel(wx, wy, wz, seed)) return 1;
    return 0;
}
