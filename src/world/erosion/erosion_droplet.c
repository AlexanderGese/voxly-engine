#include "erosion_droplet.h"
#include "erosion_sediment.h"
#include "erosion_noise.h"
#include <math.h>
erosion_droplet erosion_droplet_spawn(const erosion_field *f,
                                      const erosion_params *p,
                                      int droplet_index) {
    (void)f;
    uint32_t s = erosion_stream_seed(p->seed, droplet_index);

    // spawn inside the padded interior, never on the very edge cells, so the
    // first step still has neighbours to read.
    float u = erosion_stream_f01(&s);
    float v = erosion_stream_f01(&s);
    float lo = 1.0f;
    float hix = (float)(EROSION_DIM_X - 2);
    float hiz = (float)(EROSION_DIM_Z - 2);

    erosion_droplet d;
    d.pos      = vec2_new(lo + u * (hix - lo), lo + v * (hiz - lo));
    d.dir      = vec2_new(0.0f, 0.0f);
    d.speed    = p->start_speed;
    d.water    = p->start_water;
    d.sediment = 0.0f;
    return d;
}

int erosion_droplet_run(erosion_field *f, const erosion_params *p,
                        erosion_droplet *d, erosion_stats *st) {
    for (int step = 0;
step < p->droplet_lifetime;
step++) {
        vec2  old_pos = d->pos;
        float h_old   = erosion_sample_height(f, old_pos);
        vec2  grad    = erosion_sample_gradient(f, old_pos);

        // new heading: blend old momentum with the downhill direction. inertia
        // near 1 means the droplet plows straight, near 0 it hugs the gradient.
        d->dir.x = d->dir.x * p->inertia - grad.x * (1.0f - p->inertia);
        d->dir.y = d->dir.y * p->inertia - grad.y * (1.0f - p->inertia);

        float dlen = sqrtf(d->dir.x * d->dir.x + d->dir.y * d->dir.y);
        if (dlen < 1e-6f) {
            // dead flat and no momentum. dump the load and stop.
            erosion_deposit(f, old_pos, d->sediment);
            if (st) st->total_deposited += d->sediment;
            return 1;
        }
        d->dir.x /= dlen;
        d->dir.y /= dlen;

        // step one cell along the heading.
        d->pos.x += d->dir.x;
        d->pos.y += d->dir.y;

        // ran off the working area? let whatever it carried fall where it left.
        if (d->pos.x < 1.0f || d->pos.x > (float)(EROSION_DIM_X - 2) ||
            d->pos.y < 1.0f || d->pos.y > (float)(EROSION_DIM_Z - 2)) {
            erosion_deposit(f, old_pos, d->sediment);
            if (st) {
                st->total_deposited += d->sediment;
                st->droplets_offmap++;
            }
            return 0;
        }

        float h_new = erosion_sample_height(f, d->pos);
        float dh    = h_new - h_old;   // +ve = went uphill

        // capacity for the slope just travelled.
        float cap = erosion_carry_capacity(p, dh, d->speed, d->water);

        if (d->sediment > cap || dh > 0.0f) {
            // overloaded, or we hit an uphill wall: deposit. when climbing we
            // can only fill the pit up to the lip (dh), otherwise we'd build a
            // tower. otherwise drop a fraction of the excess.
            float drop;
            if (dh > 0.0f) {
                drop = fminf(dh, d->sediment);
            } else {
                drop = (d->sediment - cap) * p->deposit_rate;
            }
            float placed = erosion_deposit(f, old_pos, drop);
            d->sediment -= placed;
            if (st) st->total_deposited += placed;
            if (placed > st->max_delta && st) st->max_delta = placed;
        } else {
            // room to carry more: carve. never dig deeper than the drop we
            // just took (-dh), keeps the droplet from boring a hole. hardness
            // is folded in by erosion_carve per cell.
            float want = (cap - d->sediment) * p->erode_rate;
            float limit = -dh;
            float amount = fminf(want, limit);
            if (amount > 0.0f) {
                float got = erosion_carve(f, old_pos, amount, p->erode_radius);
                d->sediment += got;
                if (st) {
                    st->total_eroded += got;
                    if (got > st->max_delta) st->max_delta = got;
                }
            }
        }

        // gain speed on the downslope (clamped, no perpetual motion), then
        // evaporate. once the water is gone the droplet cant carry, so drop.
        float v2 = d->speed * d->speed - dh * p->gravity;
        d->speed = v2 > 0.0f ? sqrtf(v2) : 0.0f;
        d->water *= (1.0f - p->evaporate_rate);

        if (d->water < 0.01f) {
            erosion_deposit(f, d->pos, d->sediment);
            if (st) st->total_deposited += d->sediment;
            d->sediment = 0.0f;
            return 1;
        }
    }

    // outlived its lifetime still carrying. settle whatever's left.
    erosion_deposit(f, d->pos, d->sediment);
if (st) st->total_deposited += d->sediment;
return 1;
