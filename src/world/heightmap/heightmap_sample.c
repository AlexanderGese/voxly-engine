#include "heightmap_sample.h"
#include "heightmap_noise.h"

#include <math.h>

// each field gets its own seed salt so they dont correlate. the salts are
// arbitrary, they only need to differ.
#define SALT_CONT   0x00C0u
#define SALT_EROS   0x0E20u
#define SALT_PV     0x0BD0u
#define SALT_WEIRD  0x00DDu

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float heightmap_sample_continental(const heightmap_params *p, int wx, int wz) {
    float x = (float)wx / p->cont_scale;
    float z = (float)wz / p->cont_scale;
    // warped so coastlines wobble instead of running in straight noise bands
    float n = heightmap_warp2(x, z, p->seed ^ SALT_CONT, p->cont_warp,
                              p->cont_octaves);
    return clampf(n, -1.0f, 1.0f);
}

float heightmap_sample_erosion(const heightmap_params *p, int wx, int wz) {
    float x = (float)wx / p->eros_scale;
    float z = (float)wz / p->eros_scale;
    float n = heightmap_fbm2(x, z, p->seed ^ SALT_EROS,
                             p->eros_octaves, 2.0f, 0.5f);
    return clampf(n, -1.0f, 1.0f);
}

float heightmap_sample_peaks_valleys(const heightmap_params *p, int wx, int wz) {
    float x = (float)wx / p->pv_scale;
    float z = (float)wz / p->pv_scale;
    // ridged so the hills come to actual peaks instead of rolling lumps
    float n = heightmap_ridged2(x, z, p->seed ^ SALT_PV,
                                p->pv_octaves, 2.0f, 0.5f);
    return clampf(n, -1.0f, 1.0f);
}

float heightmap_sample_weirdness(const heightmap_params *p, int wx, int wz) {
    float x = (float)wx / p->weird_scale;
    float z = (float)wz / p->weird_scale;
    float n = heightmap_fbm2(x, z, p->seed ^ SALT_WEIRD, 3, 2.0f, 0.5f);
    return clampf(n, -1.0f, 1.0f);
}

void heightmap_sample_fields(const heightmap_params *p, int wx, int wz,
                             heightmap_fields *out) {
    out->continentalness = heightmap_sample_continental(p, wx, wz);
    out->erosion         = heightmap_sample_erosion(p, wx, wz);
    float pv             = heightmap_sample_peaks_valleys(p, wx, wz);
    out->weirdness       = heightmap_sample_weirdness(p, wx, wz);

    // weirdness folds the pv field. where weirdness is high we flip the sign so
    // a patch of "should be valley" becomes ridge and vice versa. keeps the
    // ridged noise from looking too uniform across a whole region.
    if (out->weirdness > 0.55f) {
        float t = (out->weirdness - 0.55f) / 0.45f;   // 0..1 over the top band
        pv = pv * (1.0f - t) + (-pv) * t;
    }
    out->peaks_valleys = pv;
}
