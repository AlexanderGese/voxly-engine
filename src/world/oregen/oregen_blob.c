#include "oregen_blob.h"
#include "oregen_table.h"
#include "oregen_rand.h"
#include "oregen_noise.h"

// stamp a fuzzy ellipsoid centered at (cx,cy,cz). y is squished by squish so
// horizontal spread > vertical. edge is fuzzed by a per-voxel hash compared
// against how deep inside the surface we are, which gives lumpy borders
// instead of clean math spheres. returns cells emitted.
static int stamp_ellipsoid(oregen_buf *buf, const oregen_vein *v,
                           int cx, int cy, int cz, float radius, float squish) {
    int n = 0;
    if (radius < 0.5f) radius = 0.5f;

    // warp the effective radius a touch wider so the noise has room to bite
    // back in without shrinking the blob below its budget.
    int ri = (int)(radius + 1.5f);
    float r2 = radius * radius;
    // vertical scale: dividing the y term by squish^2 makes the ellipsoid
    // taller when squish<1... we want the opposite, so invert.
    float ys = squish > 0.05f ? (1.0f / squish) : 8.0f;

    for (int dy = -ri; dy <= ri; dy++) {
        for (int dz = -ri; dz <= ri; dz++) {
            for (int dx = -ri; dx <= ri; dx++) {
                float fx = (float)dx;
                float fy = (float)dy * ys;     // squished axis
                float fz = (float)dz;
                float d2 = fx * fx + fy * fy + fz * fz;

                // noise-warped surface: push the boundary in and out by a
                // fraction of the radius so the rim is cauliflowered, not a
                // clean shell. this is the whole reason it reads as ore.
                float off = oregen_noise_surface(cx, cy, cz, dx, dy, dz,
                                                 radius * 0.45f, v->seed);
                float eff = radius + off;
                if (eff < 0.5f) eff = 0.5f;
                if (d2 > eff * eff) continue;

                // a final coin flip on the very outermost shell keeps single
                // stragglers from looking too regular.
                float edge = d2 / r2;            // 0 center .. 1 surface
                if (edge > 0.80f) {
                    float keep = 1.0f - (edge - 0.80f) / 0.40f;
                    float h = oregen_hash_f01(cx + dx, cy + dy, cz + dz,
                                              v->seed + 0x4d2u);
                    if (h > keep) continue;
                }

                if (!oregen_buf_add(buf, cx + dx, cy + dy, cz + dz, v->block))
                    return n;                    // buffer full, bail clean
                n++;
            }
        }
    }
    return n;
}

int oregen_blob_sphere_cluster(oregen_buf *buf, const oregen_vein *v) {
    const oregen_ore *ore = oregen_table_at(v->kind);
    oregen_rng rr;
    oregen_rng_seed(&rr, v->seed ^ 0x51ed270bu);

    // walk a short jittered path, stamping a shrinking sphere at each step
    // until we've roughly spent the voxel budget. nodes overlap so the
    // whole thing reads as one connected lump.
    int budget = v->size;
    int emitted = 0;

    float x = (float)v->cx, y = (float)v->cy, z = (float)v->cz;
    int nodes = 1 + v->size / 6;
    if (nodes < 1) nodes = 1;
    if (nodes > 6) nodes = 6;

    for (int i = 0; i < nodes && emitted < budget + 4; i++) {
        // each node is a fraction of the base radius, biggest in the middle.
        float t = nodes > 1 ? (float)i / (float)(nodes - 1) : 0.5f;
        float tent = 1.0f - (t < 0.5f ? (0.5f - t) : (t - 0.5f)) * 1.2f;
        float r = v->radius * (0.55f + 0.45f * tent);

        emitted += stamp_ellipsoid(buf, v, (int)x, (int)y, (int)z, r, ore->squish);
        if (oregen_buf_full(buf)) break;

        // step the path. short hops so nodes stay overlapping.
        x += oregen_rng_frange(&rr, -1.4f, 1.4f);
        z += oregen_rng_frange(&rr, -1.4f, 1.4f);
        y += oregen_rng_frange(&rr, -1.0f, 1.0f) * ore->squish;
    }
    return emitted;
}

int oregen_blob_worm(oregen_buf *buf, const oregen_vein *v) {
    const oregen_ore *ore = oregen_table_at(v->kind);
    oregen_rng rr;
    oregen_rng_seed(&rr, v->seed ^ 0x1b873593u);

    // a wandering thin tube. length scales with the budget, thickness stays
    // small. direction has momentum so it snakes instead of jittering in
    // place.
    int steps = v->size;
    if (steps < 2) steps = 2;

    float x = (float)v->cx, y = (float)v->cy, z = (float)v->cz;
    // initial heading, mostly horizontal.
    float dirx = oregen_rng_frange(&rr, -1.0f, 1.0f);
    float dirz = oregen_rng_frange(&rr, -1.0f, 1.0f);
    float diry = oregen_rng_frange(&rr, -0.4f, 0.4f);

    int emitted = 0;
    for (int i = 0; i < steps; i++) {
        // thickness wobbles a little along the length.
        float th = 0.7f + 0.5f * oregen_hash_f01((int)x, (int)y, (int)z, v->seed);
        emitted += stamp_ellipsoid(buf, v, (int)x, (int)y, (int)z, th, ore->squish);
        if (oregen_buf_full(buf)) break;

        // nudge heading, then advance. normalize-ish by clamping.
        dirx += oregen_rng_frange(&rr, -0.5f, 0.5f);
        dirz += oregen_rng_frange(&rr, -0.5f, 0.5f);
        diry += oregen_rng_frange(&rr, -0.25f, 0.25f) * ore->squish;
        if (dirx >  1.0f) dirx =  1.0f; if (dirx < -1.0f) dirx = -1.0f;
        if (dirz >  1.0f) dirz =  1.0f; if (dirz < -1.0f) dirz = -1.0f;
        if (diry >  0.6f) diry =  0.6f; if (diry < -0.6f) diry = -0.6f;

        x += dirx; y += diry; z += dirz;
    }
    return emitted;
}

int oregen_blob_pocket(oregen_buf *buf, const oregen_vein *v) {
    const oregen_ore *ore = oregen_table_at(v->kind);
    // pockets are just one tight ellipsoid. for size 1-2 ores this lands as
    // a single block or a tiny clump, which is exactly the scattered look
    // we want for diamond/emerald.
    float r = v->radius;
    if (v->size <= 2) r = 0.7f;     // keep singletons singletons
    return stamp_ellipsoid(buf, v, v->cx, v->cy, v->cz, r, ore->squish);
}

int oregen_blob_build(oregen_buf *buf, const oregen_vein *v) {
    if (!buf || !v) return 0;
    switch (v->shape) {
        case OREGEN_SHAPE_BLOB:   return oregen_blob_sphere_cluster(buf, v);
        case OREGEN_SHAPE_VEIN:   return oregen_blob_worm(buf, v);
        case OREGEN_SHAPE_POCKET: return oregen_blob_pocket(buf, v);
        default:                  return oregen_blob_sphere_cluster(buf, v);
    }
}
