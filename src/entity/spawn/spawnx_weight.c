#include "spawnx_weight.h"

// per-biome rosters for the placed/event paths. these are intentionally smaller
// and spikier than the ambient mspawn rosters: spawner blocks and sieges lean
// hostile and want a bit of variety, not the full ecosystem. costs make the
// nastier mobs rarer within a fixed budget.
//
// y bands are generous; the placed paths usually spawn near a known structure so
// the band mostly exists to keep cave-only mobs out of surface events.

static const spawnx_weight_opt OPT_PLAINS[] = {
    { ET_ZOMBIE,   1.0f, 1, 0, 128 },
    { ET_SKELETON, 0.8f, 1, 0, 128 },
    { ET_SPIDER,   0.5f, 2, 0, 128 },
};

static const spawnx_weight_opt OPT_FOREST[] = {
    { ET_ZOMBIE,   1.0f, 1, 0, 128 },
    { ET_SPIDER,   0.9f, 2, 0, 128 },
    { ET_SKELETON, 0.6f, 1, 0, 128 },
};

static const spawnx_weight_opt OPT_DESERT[] = {
    { ET_SKELETON, 1.2f, 1, 0, 128 },   // dry bones country
    { ET_ZOMBIE,   0.7f, 1, 0, 128 },
};

static const spawnx_weight_opt OPT_CAVE[] = {
    { ET_SKELETON, 1.0f, 1, 0,  48 },
    { ET_ZOMBIE,   1.0f, 1, 0,  48 },
    { ET_SPIDER,   0.8f, 2, 0,  48 },
};

// most biomes just reuse plains/forest/cave; we only special-case the spicy
// ones. swamp leans spider, tundra leans skeleton, etc, but plains covers them
// well enough that i didnt bother authoring a row for each.
spawnx_roster spawnx_weight_biome(biome_id biome) {
    spawnx_roster r;
    r.budget = 0;
    switch (biome) {
        case BIOME_FOREST:
        case BIOME_SWAMP:
            r.opt = OPT_FOREST;  r.count = 3; break;
        case BIOME_DESERT:
        case BIOME_BEACH:
            r.opt = OPT_DESERT;  r.count = 2; break;
        case BIOME_MOUNTAINS:
        case BIOME_TUNDRA:
            r.opt = OPT_CAVE;    r.count = 3; break;
        case BIOME_PLAINS:
        default:
            r.opt = OPT_PLAINS;  r.count = 3; break;
    }
    return r;
}

// affordable + in y band. shared predicate so total and draw agree exactly.
static int opt_eligible(const spawnx_roster *r, const spawnx_weight_opt *o,
                        int wy) {
    if (o->weight <= 0.0f) return 0;
    if (o->cost > r->budget) return 0;
    if (wy < o->min_y || wy > o->max_y) return 0;
    return 1;
}

float spawnx_weight_total(const spawnx_roster *r, int wy) {
    float total = 0.0f;
    for (int i = 0; i < r->count; i++) {
        if (opt_eligible(r, &r->opt[i], wy)) total += r->opt[i].weight;
    }
    return total;
}

int spawnx_weight_draw(spawnx_roster *r, mspawn_rng *rng, int wy,
                       entity_type *out) {
    float total = spawnx_weight_total(r, wy);
    if (total <= 0.0f) return 0;

    // cumulative scan: roll into [0,total), walk until we cross the threshold.
    float roll = mspawn_rng_f01(rng) * total;
    float acc = 0.0f;
    for (int i = 0; i < r->count; i++) {
        const spawnx_weight_opt *o = &r->opt[i];
        if (!opt_eligible(r, o, wy)) continue;
        acc += o->weight;
        if (roll < acc) {
            r->budget -= o->cost;
            *out = o->type;
            return 1;
        }
    }
    // float slop could leave us just past the end; hand back the last eligible.
    for (int i = r->count - 1; i >= 0; i--) {
        const spawnx_weight_opt *o = &r->opt[i];
        if (!opt_eligible(r, o, wy)) continue;
        r->budget -= o->cost;
        *out = o->type;
        return 1;
    }
    return 0;
}
