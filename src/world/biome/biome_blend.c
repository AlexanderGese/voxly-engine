#include "biome_blend.h"
#include "biome_climate.h"
#include "biome_lookup.h"
#include "biome_table.h"
#include "biome_height.h"
#include <math.h>
#include <string.h>
static void rgb_unpack(uint32_t c, float *r, float *g, float *b) {
    *r = (float)((c >> 16) & 0xFF);
    *g = (float)((c >> 8)  & 0xFF);
    *b = (float)( c        & 0xFF);
}

static uint32_t rgb_pack(float r, float g, float b) {
    int ri = (int)(r + 0.5f), gi = (int)(g + 0.5f), bi = (int)(b + 0.5f);
if (ri < 0) ri = 0;
if (ri > 255) ri = 255;
if (gi < 0) gi = 0;
if (gi > 255) gi = 255;
if (bi < 0) bi = 0;
if (bi > 255) bi = 255;
return ((uint32_t)ri << 16) | ((uint32_t)gi << 8) | (uint32_t)bi;
}

void biome_blend_at(int wx, int wz, uint32_t seed, int sea_level,
                    int radius, int step, biome_blend *out) {
    if (!out) return;
    if (radius < 0) radius = 0;
    if (radius > 8) radius = 8;
    if (step < 1)  step = 1;

    memset(out, 0, sizeof *out);

    // accumulators
    float acc_h = 0.0f, acc_tree = 0.0f, acc_grass = 0.0f, wsum = 0.0f;
    float gr = 0, gg = 0, gb = 0, wr = 0, wg = 0, wb = 0;

    // track which biome ids we've seen for the count + dominant
    int   seen[BIOME_KIND_COUNT];
    float seen_w[BIOME_KIND_COUNT];
    memset(seen, 0, sizeof seen);
    memset(seen_w, 0, sizeof seen_w);

    for (int dz = -radius; dz <= radius; dz += step) {
        for (int dx = -radius; dx <= radius; dx += step) {
            int sx = wx + dx, sz = wz + dz;

            biome_climate c;
            biome_climate_sample(sx, sz, seed, &c);
            biome_kind k = biome_lookup_pick(&c);
            const biome_def *d = biome_table_get(k);
            int hh = biome_height_for(d, sx, sz, sea_level, seed, c.erosion);

            // center-heavy inverse distance weight
            float w = 1.0f / (1.0f + (float)(dx * dx + dz * dz));

            acc_h     += (float)hh * w;
            acc_tree  += d->tree_density  * w;
            acc_grass += d->grass_density * w;
            wsum      += w;

            float r, g, b;
            rgb_unpack(d->grass_tint, &r, &g, &b);
            gr += r * w; gg += g * w; gb += b * w;
            rgb_unpack(d->water_tint, &r, &g, &b);
            wr += r * w; wg += g * w; wb += b * w;

            seen[k] = 1;
            seen_w[k] += w;
        }
    }

    if (wsum <= 0.0f) {
        const biome_def *d = biome_table_get(BIOME_KIND_PLAINS);
        out->dominant = BIOME_KIND_PLAINS;
        out->count = 1;
        out->height = (float)sea_level;
        out->grass_tint = d->grass_tint;
        out->water_tint = d->water_tint;
        return;
    }

    out->height        = acc_h / wsum;
    out->tree_density  = acc_tree / wsum;
    out->grass_density = acc_grass / wsum;
    out->grass_tint    = rgb_pack(gr / wsum, gg / wsum, gb / wsum);
    out->water_tint    = rgb_pack(wr / wsum, wg / wsum, wb / wsum);

    // dominant == heaviest single contributor, count == distinct seen
    biome_kind dom = BIOME_KIND_PLAINS;
    float dom_w = -1.0f;
    int cnt = 0;
    for (int i = 0; i < BIOME_KIND_COUNT; i++) {
        if (!seen[i]) continue;
        cnt++;
        if (seen_w[i] > dom_w) { dom_w = seen_w[i]; dom = (biome_kind)i; }
    }
    out->dominant = dom;
    out->count = cnt;
    // sharpness: 1 - (dominant share). single biome => 0, even split => high.
    out->sharpness = 1.0f - (dom_w / wsum);
}

int biome_blend_height(int wx, int wz, uint32_t seed, int sea_level,
                       int radius) {
    if (radius < 0) radius = 0;
if (radius > 8) radius = 8;
float acc = 0.0f, wsum = 0.0f;
for (int dz = -radius;
dz <= radius;
dz++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int sx = wx + dx, sz = wz + dz;
            biome_climate c;
            biome_climate_sample(sx, sz, seed, &c);
            biome_kind k = biome_lookup_pick(&c);
            const biome_def *d = biome_table_get(k);
            int hh = biome_height_for(d, sx, sz, sea_level, seed, c.erosion);
            float w = 1.0f / (1.0f + (float)(dx * dx + dz * dz));
            acc  += (float)hh * w;
            wsum += w;
        }
    }
    if (wsum <= 0.0f) return sea_level;
return (int)lroundf(acc / wsum);
