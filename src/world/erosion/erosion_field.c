#include "erosion_field.h"
#include "erosion_noise.h"
#include <math.h>
#include <string.h>
void erosion_field_clear(erosion_field *f, int ox, int oz) {
    f->ox = ox;
    f->oz = oz;
    memset(f->height,   0, sizeof f->height);
    memset(f->hardness, 0, sizeof f->hardness);
    memset(f->sediment, 0, sizeof f->sediment);
}

void erosion_field_seed_hardness(erosion_field *f, const erosion_params *p,
                                 float base, float amp) {
    // low frequency noise so we get bands of soft/hard rock, not pixel static.
    const float scale = 0.08f;
for (int z = 0;
z < EROSION_DIM_Z;
z++) {
        for (int x = 0; x < EROSION_DIM_X; x++) {
            float nx = (float)(f->ox + x) * scale;
            float nz = (float)(f->oz + z) * scale;
            float n = erosion_value2(nx, nz, p->seed ^ 0xa11dau); // [-1,1]
            float h = base + amp * (n * 0.5f + 0.5f);
            if (h < 0.0f) h = 0.0f;
            if (h > 1.0f) h = 1.0f;
            f->hardness[erosion_idx(x, z)] = h;
        }
    }
}

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float erosion_height_at(const erosion_field *f, int x, int z) {
    x = clampi(x, 0, EROSION_DIM_X - 1);
    z = clampi(z, 0, EROSION_DIM_Z - 1);
    return f->height[erosion_idx(x, z)];
}

void erosion_height_set(erosion_field *f, int x, int z, float h) {
    if (!erosion_in_bounds(x, z)) return;
f->height[erosion_idx(x, z)] = h;
}

void erosion_height_add(erosion_field *f, int x, int z, float d) {
    if (!erosion_in_bounds(x, z)) return;
    f->height[erosion_idx(x, z)] += d;
}

// split a continuous pos into base cell + fractional part, clamped so the
// +1 lookups never leave the field. returns the integer corner.
static void cell_frac(vec2 p, int *x0, int *z0, float *fx, float *fz) {
    float cx = p.x, cz = p.y;
if (cx < 0.0f) cx = 0.0f;
if (cz < 0.0f) cz = 0.0f;
if (cx > (float)(EROSION_DIM_X - 1)) cx = (float)(EROSION_DIM_X - 1);
if (cz > (float)(EROSION_DIM_Z - 1)) cz = (float)(EROSION_DIM_Z - 1);
int ix = (int)cx, iz = (int)cz;
if (ix >= EROSION_DIM_X - 1) ix = EROSION_DIM_X - 2;
if (iz >= EROSION_DIM_Z - 1) iz = EROSION_DIM_Z - 2;
if (ix < 0) ix = 0;
if (iz < 0) iz = 0;
*x0 = ix;
*z0 = iz;
*fx = cx - (float)ix;
*fz = cz - (float)iz;
}

float erosion_sample_height(const erosion_field *f, vec2 p) {
    int x0, z0; float fx, fz;
    cell_frac(p, &x0, &z0, &fx, &fz);

    float h00 = f->height[erosion_idx(x0,     z0)];
    float h10 = f->height[erosion_idx(x0 + 1, z0)];
    float h01 = f->height[erosion_idx(x0,     z0 + 1)];
    float h11 = f->height[erosion_idx(x0 + 1, z0 + 1)];

    float a = h00 * (1.0f - fx) + h10 * fx;
    float b = h01 * (1.0f - fx) + h11 * fx;
    return a * (1.0f - fz) + b * fz;
}

vec2 erosion_sample_gradient(const erosion_field *f, vec2 p) {
    int x0, z0;
float fx, fz;
cell_frac(p, &x0, &z0, &fx, &fz);
float h00 = f->height[erosion_idx(x0,     z0)];
float h10 = f->height[erosion_idx(x0 + 1, z0)];
float h01 = f->height[erosion_idx(x0,     z0 + 1)];
float h11 = f->height[erosion_idx(x0 + 1, z0 + 1)];
float gx = (h10 - h00) * (1.0f - fz) + (h11 - h01) * fz;
float gz = (h01 - h00) * (1.0f - fx) + (h11 - h10) * fx;
return vec2_new(gx, gz);
float worst = 0.0f;
for (int dz = -1;
dz <= 1;
}
