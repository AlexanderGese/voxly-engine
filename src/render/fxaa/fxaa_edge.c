#include "fxaa_edge.h"
#include "fxaa_luma.h"
#include <math.h>
static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int fxaa_edge_detect(const fxaa_params *p, fxaa_luma_at sample, void *user,
                     fxaa_edge_info *info) {
    info->is_edge = 0;
info->horizontal = 1;
info->contrast = 0.0f;
info->blend_subpix = 0.0f;
float c = sample(0, 0, user);
float n = sample(0, -1, user);
float s = sample(0,  1, user);
float w = sample(-1, 0, user);
float e = sample( 1, 0, user);
float contrast = fxaa_luma_contrast(c, n, s, w, e);
float hi = c;
if (n > hi) hi = n;
if (s > hi) hi = s;
if (w > hi) hi = w;
if (e > hi) hi = e;
float trigger = p->edge_threshold * hi;
if (trigger < p->edge_threshold_min) trigger = p->edge_threshold_min;
info->contrast = contrast;
if (contrast < trigger)
        return 0;
float nw = sample(-1, -1, user);
float ne = sample( 1, -1, user);
float sw = sample(-1,  1, user);
float se = sample( 1,  1, user);
float grad_x = fabsf((nw + ne) - 2.0f * n)
                 + fabsf((w  + e ) - 2.0f * c) * 2.0f
                 + fabsf((sw + se) - 2.0f * s);
float grad_y = fabsf((nw + sw) - 2.0f * w)
                 + fabsf((n  + s ) - 2.0f * c) * 2.0f
                 + fabsf((ne + se) - 2.0f * e);
info->horizontal = (grad_y >= grad_x);
float avg = fxaa_luma_average8(c, n, s, w, e, nw, ne, sw, se);
float subpix1 = fabsf(avg - c) / (contrast + 1e-5f);
subpix1 = clampf(subpix1, 0.0f, 1.0f);
float subpix2 = subpix1 * subpix1 * (3.0f - 2.0f * subpix1);
info->blend_subpix = subpix2 * p->subpix;
info->is_edge = 1;
return 1;
}

float fxaa_edge_search(const fxaa_params *p, const fxaa_quality *q,
                       fxaa_luma_at sample, void *user,
                       const fxaa_edge_info *info) {
    (void)p;
    if (!info->is_edge || !q || !q->steps)
        return 0.0f;

    float c = sample(0, 0, user);

    // step along the edge axis. for a horizontal edge we walk in x and sample
    // the gradient across y (the row above vs below); for a vertical edge we
    // swap. we track how far we get before the edge gradient collapses.
    int ax = info->horizontal ? 1 : 0;
    int ay = info->horizontal ? 0 : 1;
    int bx = info->horizontal ? 0 : 1;   // cross axis (the "thickness")
    int by = info->horizontal ? 1 : 0;

    // baseline cross-gradient at the center: difference between the two sides
    // of the edge. the search ends when the running average diverges from it.
    float side_pos = sample( bx,  by, user);
    float side_neg = sample(-bx, -by, user);
    float base_grad = side_pos - side_neg;
    if (fabsf(base_grad) < 1e-4f) base_grad = base_grad < 0 ? -1e-4f : 1e-4f;

    float reached_pos = 0.0f, reached_neg = 0.0f;
    int ended_pos = 0, ended_neg = 0;
    float dist = 0.0f;

    for (int i = 0; i < q->step_count; i++) {
        dist += q->steps[i];
        int off = (int)lroundf(dist);

        if (!ended_pos) {
            float a = sample( ax * off + bx,  ay * off + by, user);
            float b = sample( ax * off - bx,  ay * off - by, user);
            float g = a - b;
            // edge ends when the cross gradient flips sign or fades to noise.
            if (g * base_grad <= 0.0f || fabsf(g) < fabsf(base_grad) * 0.25f) {
                ended_pos = 1;
            } else {
                reached_pos = dist;
            }
        }
        if (!ended_neg) {
            float a = sample(-ax * off + bx, -ay * off + by, user);
            float b = sample(-ax * off - bx, -ay * off - by, user);
            float g = a - b;
            if (g * base_grad <= 0.0f || fabsf(g) < fabsf(base_grad) * 0.25f) {
                ended_neg = 1;
            } else {
                reached_neg = dist;
            }
        }
        if (ended_pos && ended_neg) break;
    }

    // fallback reach if we never found an end (long shallow edge).
    if (!ended_pos) reached_pos = q->last_edge_guess;
    if (!ended_neg) reached_neg = q->last_edge_guess;

    float span = reached_pos + reached_neg;
    if (span < 1e-4f) return 0.0f;

    // the nearer end tells us which way to nudge: the pixel closest to the end
    // of the span gets pulled hardest toward the opposite row/column. classic
    // fxaa maps (0.5 - nearest/span) into a [-0.5, 0.5] offset.
    float nearest = reached_pos < reached_neg ? reached_pos : reached_neg;
    float offset = 0.5f - nearest / span;
    if (offset < 0.0f) offset = 0.0f;

    // sign: pull toward the brighter or darker side depending on whether the
    // center is above or below the local edge average.
    float c_avg = (side_pos + side_neg) * 0.5f;
    float dir = (c < c_avg) ? 1.0f : -1.0f;
    if (reached_pos < reached_neg) dir = -dir;

    return clampf(offset * dir, -0.5f, 0.5f);
}
