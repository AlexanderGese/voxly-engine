#include "heightmap_sample.h"
#include "heightmap_noise.h"
#include <math.h>
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
float n = heightmap_ridged2(x, z, p->seed ^ SALT_PV,
                                p->pv_octaves, 2.0f, 0.5f);
return clampf(n, -1.0f, 1.0f);
out->erosion         = heightmap_sample_erosion(p, wx, wz);
float pv             = heightmap_sample_peaks_valleys(p, wx, wz);
out->weirdness       = heightmap_sample_weirdness(p, wx, wz);
}
