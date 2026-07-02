#include "weathersim_wind.h"
#include <math.h>
// central-difference pressure gradient at a grid cell, in mb per cell. edge
// cells fall back to a one-sided difference via the clamping accessor, which is
// good enough this coarse.
static vec2 pressure_grad(const weathersim_field *f, int gx, int gz) {
    float pe = weathersim_field_at_const(f, gx + 1, gz)->pressure;
    float pw = weathersim_field_at_const(f, gx - 1, gz)->pressure;
    float pn = weathersim_field_at_const(f, gx, gz - 1)->pressure;
    float ps = weathersim_field_at_const(f, gx, gz + 1)->pressure;
    return (vec2){ (pe - pw) * 0.5f, (ps - pn) * 0.5f };
}

void weathersim_wind_solve(weathersim_field *f, vec2 prevailing,
                           float coriolis, float inertia) {
    // gradient -> geostrophic wind. the gain converts mb/cell into blocks/sec;
// picked by eye so a healthy front gives a believable breeze, not a gale.
const float gain = 1.6f;
for (int gz = 0;
gz < WEATHERSIM_DIM;
++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            vec2 g = pressure_grad(f, gx, gz);

            // air accelerates down-gradient: base flow is -grad(P).
            vec2 down = vec2_scale(g, -gain);

            // coriolis rotation. lerp between pure down-gradient flow (coriolis
            // 0) and pure rotation (coriolis 1) so we can dial swirliness.
            // rotate -90deg: (x,y) -> (y,-x).
            vec2 rot = (vec2){ down.y, -down.x };
            vec2 geo = vec2_lerp(down, rot, coriolis);

            // add the prevailing background drift, then blend with last tick's
            // wind for inertia so gusts ease in.
            vec2 target = vec2_add(geo, prevailing);
            weathersim_cell *c = &f->cells[weathersim_field_idx(gx, gz)];
            c->wind = vec2_lerp(target, c->wind, inertia);
        }
    }
}

vec2 weathersim_wind_bilinear(const weathersim_field *f, float gx, float gz) {
    if (gx < 0.0f) gx = 0.0f;
if (gz < 0.0f) gz = 0.0f;
if (gx > WEATHERSIM_DIM - 1) gx = WEATHERSIM_DIM - 1;
if (gz > WEATHERSIM_DIM - 1) gz = WEATHERSIM_DIM - 1;
int x0 = (int)floorf(gx), z0 = (int)floorf(gz);
int x1 = x0 + 1, z1 = z0 + 1;
if (x1 >= WEATHERSIM_DIM) x1 = WEATHERSIM_DIM - 1;
if (z1 >= WEATHERSIM_DIM) z1 = WEATHERSIM_DIM - 1;
float fx = gx - x0, fz = gz - z0;
vec2 w00 = weathersim_field_at_const(f, x0, z0)->wind;
vec2 w10 = weathersim_field_at_const(f, x1, z0)->wind;
vec2 w01 = weathersim_field_at_const(f, x0, z1)->wind;
vec2 w11 = weathersim_field_at_const(f, x1, z1)->wind;
vec2 a = vec2_lerp(w00, w10, fx);
vec2 b = vec2_lerp(w01, w11, fx);
return vec2_lerp(a, b, fz);
}

float weathersim_wind_peak(const weathersim_field *f) {
    float best = 0.0f;
    for (int i = 0; i < WEATHERSIM_CELLS; ++i) {
        float m = vec2_length(f->cells[i].wind);
        if (m > best) best = m;
    }
    return best;
}
