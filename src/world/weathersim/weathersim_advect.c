#include "weathersim_advect.h"

#include <math.h>
#include <string.h>

// bilinear gather of (temp, humidity, cloud) at a fractional grid position.
// edge-clamped — air tracing back off the window just samples the boundary
// cell, which reads as "more of the same blowing in", good enough at the rim.
static void sample_scalars(const weathersim_field *f, float gx, float gz,
                           float *temp, float *hum, float *cloud) {
    if (gx < 0.0f) gx = 0.0f;
    if (gz < 0.0f) gz = 0.0f;
    if (gx > WEATHERSIM_DIM - 1) gx = WEATHERSIM_DIM - 1;
    if (gz > WEATHERSIM_DIM - 1) gz = WEATHERSIM_DIM - 1;

    int x0 = (int)floorf(gx), z0 = (int)floorf(gz);
    int x1 = x0 + 1, z1 = z0 + 1;
    if (x1 >= WEATHERSIM_DIM) x1 = WEATHERSIM_DIM - 1;
    if (z1 >= WEATHERSIM_DIM) z1 = WEATHERSIM_DIM - 1;
    float fx = gx - x0, fz = gz - z0;

    const weathersim_cell *c00 = weathersim_field_at_const(f, x0, z0);
    const weathersim_cell *c10 = weathersim_field_at_const(f, x1, z0);
    const weathersim_cell *c01 = weathersim_field_at_const(f, x0, z1);
    const weathersim_cell *c11 = weathersim_field_at_const(f, x1, z1);

    // lerp helper inlined three times rather than a macro; the compiler folds
    // it and it keeps the rounding identical across the members.
    float t0 = c00->temp + (c10->temp - c00->temp) * fx;
    float t1 = c01->temp + (c11->temp - c01->temp) * fx;
    *temp = t0 + (t1 - t0) * fz;

    float h0 = c00->humidity + (c10->humidity - c00->humidity) * fx;
    float h1 = c01->humidity + (c11->humidity - c01->humidity) * fx;
    *hum = h0 + (h1 - h0) * fz;

    float k0 = c00->cloud + (c10->cloud - c00->cloud) * fx;
    float k1 = c01->cloud + (c11->cloud - c01->cloud) * fx;
    *cloud = k0 + (k1 - k0) * fz;
}

void weathersim_advect_step(weathersim_field *f, float dt) {
    // wind is blocks/sec; one cell is WEATHERSIM_BLOCKS_PER_CELL blocks. convert
    // a tick's displacement into cell units for the backtrace.
    float inv_cell = 1.0f / (float)WEATHERSIM_BLOCKS_PER_CELL;

    for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            const weathersim_cell *c = &f->cells[weathersim_field_idx(gx, gz)];

            // departure point: where this parcel of air was one tick ago.
            float bx = gx - c->wind.x * dt * inv_cell;
            float bz = gz - c->wind.y * dt * inv_cell;

            float temp, hum, cloud;
            sample_scalars(f, bx, bz, &temp, &hum, &cloud);

            weathersim_cell *o = &f->scratch[weathersim_field_idx(gx, gz)];
            *o = *c;                 // keep pressure/wind/accum/ground_t as-is
            o->temp     = temp;
            o->humidity = hum;
            o->cloud    = cloud;
        }
    }
    memcpy(f->cells, f->scratch, sizeof f->cells);
}
