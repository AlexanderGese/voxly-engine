#include "ssr_march.h"
#include "ssr_config.h"

#include <math.h>

void ssrx_march_params_default(ssrx_march_params *p) {
    p->max_steps    = SSRX_DEFAULT_STEPS;
    p->refine_steps = SSRX_DEFAULT_REFINE;
    p->stride       = SSRX_DEFAULT_STRIDE;
    p->growth       = SSRX_STRIDE_GROWTH;
    p->thickness    = SSRX_DEFAULT_THICKNESS;
    p->start_jitter = 0.0f;
}

// the total view-space distance the longest march could cover, for normalizing
// the travel into a 0..1 fade parameter. geometric series sum.
static float max_march_length(const ssrx_march_params *p) {
    if (fabsf(p->growth - 1.0f) < 1e-4f)
        return p->stride * (float)p->max_steps;
    float gN = powf(p->growth, (float)p->max_steps);
    return p->stride * (gN - 1.0f) / (p->growth - 1.0f);
}

// project a view-space point to a uv and pull the scene's view-z under it.
// returns 0 if the point left the screen or sat on the far plane.
static int sample_scene_z(const ssrx_gbuffer *g, const ssrx_depthbuf *d,
                          vec3 view_pt, vec2 *out_uv, float *out_scene_z) {
    vec2 uv;
    float ndc;
    if (!ssrx_gbuffer_project(g, view_pt, &uv, &ndc)) return 0;
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f) return 0;
    float scene_z;
    if (!ssrx_depth_scene_view_z(d, g, uv, &scene_z)) return 0;
    if (out_uv)      *out_uv = uv;
    if (out_scene_z) *out_scene_z = scene_z;
    return 1;
}

// is the marched point behind the stored surface? view-space looks down -z, so
// "behind the surface" means the point is *further* from the eye than the scene
// surface: marched_z < scene_z (more negative). the depth difference is
// (marched_z - scene_z); negative once we punch through.
static int crossed(float marched_z, float scene_z) {
    return marched_z < scene_z;
}

ssrx_march_result ssrx_march_run(const ssrx_ray *ray,
                                 const ssrx_gbuffer *g,
                                 const ssrx_depthbuf *depth,
                                 const ssrx_march_params *p) {
    ssrx_march_result out;
    out.status     = SSRX_MARCH_MISS;
    out.hit_uv     = vec2_new(0.0f, 0.0f);
    out.hit_view_z = 0.0f;
    out.travel     = 0.0f;
    out.t          = 0.0f;
    out.steps      = 0;

    if (!ray->valid) return out;

    float stride   = p->stride;
    // jitter the start by a sub-stride offset so neighbouring pixels dont share
    // step boundaries (banding). clamp so we never start behind the origin.
    float dist     = p->start_jitter * p->stride;
    if (dist < 0.0f) dist = 0.0f;
    float prev_dist = 0.0f;     // last point that was still in front

    // ---- SEEK: linear march with geometric stride ----
    for (int i = 0; i < p->max_steps; i++) {
        prev_dist = dist;
        dist += stride;
        stride *= p->growth;
        out.steps = i + 1;

        vec3 pt = ssrx_ray_at(ray, dist);

        vec2 uv;
        float scene_z;
        if (!sample_scene_z(g, depth, pt, &uv, &scene_z)) {
            // left the screen or hit sky. keep going — a later step may come
            // back on screen, but if we never re-enter it stays a miss.
            continue;
        }

        if (crossed(pt.z, scene_z)) {
            // we punched behind the surface. only count it if the surface is
            // within the thickness band; a deep gap means we tunnelled through
            // thin geometry into empty space and this is a false hit.
            float gap = scene_z - pt.z;   // how far behind we are, positive
            if (gap > p->thickness) {
                // too deep — treat as occluded, abandon the ray. (the gpu does
                // the same: thin features shouldnt reflect the wall behind.)
                return out;
            }
            // ---- REFINE: bisect between prev_dist (front) and dist (behind) ----
            float lo = prev_dist;     // in front
            float hi = dist;          // behind
            vec2  best_uv = uv;
            float best_z  = scene_z;
            for (int r = 0; r < p->refine_steps; r++) {
                float mid = 0.5f * (lo + hi);
                vec3  mp  = ssrx_ray_at(ray, mid);
                vec2  muv;
                float mz;
                if (!sample_scene_z(g, depth, mp, &muv, &mz)) {
                    // refine wandered off screen; pull back toward the front.
                    hi = mid;
                    continue;
                }
                if (crossed(mp.z, mz)) {
                    hi = mid;          // still behind, move the far bound in
                    best_uv = muv;
                    best_z  = mz;
                } else {
                    lo = mid;          // in front, move the near bound out
                }
            }

            out.status     = SSRX_MARCH_HIT;
            out.hit_uv     = best_uv;
            out.hit_view_z = best_z;
            out.travel     = 0.5f * (lo + hi);
            float maxlen   = max_march_length(p);
            out.t          = maxlen > 0.0f ? out.travel / maxlen : 1.0f;
            if (out.t > 1.0f) out.t = 1.0f;
            return out;
        }
    }

    // never crossed — miss. record how far we got for telemetry.
    out.travel = dist;
    float maxlen = max_march_length(p);
    out.t = maxlen > 0.0f ? dist / maxlen : 1.0f;
    return out;
}
