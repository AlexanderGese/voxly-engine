#include "erosion_sediment.h"

#include <math.h>

float erosion_carry_capacity(const erosion_params *p,
                             float slope, float speed, float water) {
    // negative slope means downhill, which is the only direction that should
    // grant capacity. uphill droplets are about to die so clamp to ~flat.
    float s = -slope;
    if (s < 0.05f) s = 0.05f;

    float cap = s * speed * water * p->capacity_factor;
    if (cap < p->min_capacity) cap = p->min_capacity;
    return cap;
}

float erosion_deposit(erosion_field *f, vec2 p, float amount) {
    if (amount <= 0.0f) return 0.0f;

    int x0 = (int)floorf(p.x), z0 = (int)floorf(p.y);
    float fx = p.x - (float)x0, fz = p.y - (float)z0;

    // bilinear weights, same corners we sampled height from. dropping it back
    // weighted means the droplet fills the exact dip it was sitting in.
    float w00 = (1.0f - fx) * (1.0f - fz);
    float w10 = fx          * (1.0f - fz);
    float w01 = (1.0f - fx) * fz;
    float w11 = fx          * fz;

    erosion_height_add(f, x0,     z0,     amount * w00);
    erosion_height_add(f, x0 + 1, z0,     amount * w10);
    erosion_height_add(f, x0,     z0 + 1, amount * w01);
    erosion_height_add(f, x0 + 1, z0 + 1, amount * w11);
    return amount;
}

float erosion_carve(erosion_field *f, vec2 p, float amount, float radius) {
    if (amount <= 0.0f) return 0.0f;

    int cx = (int)floorf(p.x), cz = (int)floorf(p.y);
    int r  = (int)ceilf(radius);
    if (r < 1) r = 1;

    // first pass: accumulate a gaussian-ish weight over the disc so we can
    // normalise. carve scales by (1 - hardness) per cell so a vein of hard
    // rock leaves a ridge while the soft stuff washes out around it.
    float wsum = 0.0f;
    for (int dz = -r; dz <= r; dz++) {
        for (int dx = -r; dx <= r; dx++) {
            int x = cx + dx, z = cz + dz;
            if (!erosion_in_bounds(x, z)) continue;
            float ddx = (float)x + 0.5f - p.x;
            float ddz = (float)z + 0.5f - p.y;
            float d2 = ddx * ddx + ddz * ddz;
            float rr = radius * radius;
            if (d2 > rr) continue;
            float w = 1.0f - sqrtf(d2 / rr);     // linear falloff, peak at centre
            wsum += w;
        }
    }
    if (wsum <= 0.0f) return 0.0f;

    float removed = 0.0f;
    for (int dz = -r; dz <= r; dz++) {
        for (int dx = -r; dx <= r; dx++) {
            int x = cx + dx, z = cz + dz;
            if (!erosion_in_bounds(x, z)) continue;
            float ddx = (float)x + 0.5f - p.x;
            float ddz = (float)z + 0.5f - p.y;
            float d2 = ddx * ddx + ddz * ddz;
            float rr = radius * radius;
            if (d2 > rr) continue;

            float w = 1.0f - sqrtf(d2 / rr);
            int   i = erosion_idx(x, z);
            float soft = 1.0f - f->hardness[i];   // 0 hard .. 1 soft
            float take = amount * (w / wsum) * soft;
            f->height[i] -= take;
            removed += take;
        }
    }
    return removed;
}

int erosion_settle(erosion_field *f, const erosion_params *p) {
    int settled = 0;
    for (int it = 0; it < p->settle_iters; it++) {
        for (int z = 0; z < EROSION_DIM_Z; z++) {
            for (int x = 0; x < EROSION_DIM_X; x++) {
                int i = erosion_idx(x, z);
                float s = f->sediment[i];
                if (s <= 0.0f) continue;
                if (s < p->settle_thresh) {
                    // settled for good, becomes terrain.
                    f->height[i]  += s;
                    f->sediment[i] = 0.0f;
                    settled++;
                } else {
                    // still loose, bleed a little into height so it doesnt sit
                    // forever. the rest waits for the next pass.
                    float bleed = s * 0.25f;
                    f->height[i]   += bleed;
                    f->sediment[i] -= bleed;
                }
            }
        }
    }
    return settled;
}
