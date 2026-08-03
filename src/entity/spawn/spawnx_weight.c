#include "spawnx_weight.h"
static const spawnx_weight_opt OPT_PLAINS[] = {
    { ET_ZOMBIE,   1.0f, 1, 0, 128 },
    { ET_SKELETON, 0.8f, 1, 0, 128 },
    { ET_SPIDER,   0.5f, 2, 0, 128 },
}
;
static const spawnx_weight_opt OPT_FOREST[] = {
    { ET_ZOMBIE,   1.0f, 1, 0, 128 },
    { ET_SPIDER,   0.9f, 2, 0, 128 },
    { ET_SKELETON, 0.6f, 1, 0, 128 },
}
;
static const spawnx_weight_opt OPT_DESERT[] = {
    { ET_SKELETON, 1.2f, 1, 0, 128 },   // dry bones country
    { ET_ZOMBIE,   0.7f, 1, 0, 128 },
}
;
static const spawnx_weight_opt OPT_CAVE[] = {
    { ET_SKELETON, 1.0f, 1, 0,  48 },
    { ET_ZOMBIE,   1.0f, 1, 0,  48 },
    { ET_SPIDER,   0.8f, 2, 0,  48 },
}
;
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
float roll = mspawn_rng_f01(rng) * total;
float acc = 0.0f;
for (int i = 0;
i < r->count;
i++) {
        const spawnx_weight_opt *o = &r->opt[i];
        if (!opt_eligible(r, o, wy)) continue;
        acc += o->weight;
        if (roll < acc) {
            r->budget -= o->cost;
            *out = o->type;
            return 1;
        }
    }
    // float slop could leave us just past the end;
hand back the last eligible.
    for (int i = r->count - 1;
i >= 0;
}
