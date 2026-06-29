#include "structgen_placement.h"
#include "structgen_rand.h"
#include "../../config.h"

// integer floor-div, needed because c truncates toward zero and chunk coords
// go negative. without this the region grid tears at the x=0/z=0 seam.
static int floordiv(int a, int b) {
    int q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
    return q;
}

uint32_t structgen_site_seed(structgen_kind kind, int wx, int wz, uint32_t world_seed) {
    uint32_t h = structgen_hash2(wx, wz, world_seed);
    return structgen_seed_mix(h, (uint32_t)kind * 0x9e37u + 1u);
}

int structgen_village_cell(const structgen_config *cfg, int chunk_x, int chunk_z,
                           uint32_t world_seed, int *out_cx, int *out_cz) {
    if (cfg->village_per_region <= 0) { return 0; }

    int rs = cfg->region_size;
    if (rs < 1) rs = 1;

    int rx = floordiv(chunk_x, rs);
    int rz = floordiv(chunk_z, rs);

    // jitter the village inside its region cell. keep a 1-chunk margin off the
    // edges so two neighbor villages cant end up adjacent across a seam.
    uint32_t hx = structgen_hash2(rx, rz, world_seed ^ 0x5f31a7u);
    uint32_t hz = structgen_hash2(rz, rx, world_seed ^ 0x91c2b3u);

    int span = rs - 2;
    if (span < 1) span = 1;
    int jx = 1 + (int)(hx % (uint32_t)span);
    int jz = 1 + (int)(hz % (uint32_t)span);

    int cx = rx * rs + jx;
    int cz = rz * rs + jz;

    if (out_cx) *out_cx = cx;
    if (out_cz) *out_cz = cz;
    return (cx == chunk_x && cz == chunk_z);
}

structgen_kind structgen_pick(const structgen_config *cfg,
                              int chunk_x, int chunk_z, uint32_t world_seed,
                              structgen_site *site) {
    int cx, cz;
    structgen_kind kind = STRUCTGEN_NONE;

    // villages win priority: they're rare and big, dont want a dungeon roll
    // stealing a hub chunk.
    if (structgen_village_cell(cfg, chunk_x, chunk_z, world_seed, &cx, &cz)) {
        kind = STRUCTGEN_VILLAGE;
    } else {
        // independent per-chunk rolls for the small stuff. dungeons buried,
        // ruins surface; they can coexist with villages elsewhere but not
        // on the same chunk, so we gate them behind the village miss.
        float dr = structgen_hash_f01(chunk_x, chunk_z, world_seed ^ 0xd0006eu);
        float rr = structgen_hash_f01(chunk_z, chunk_x, world_seed ^ 0x7011cu);
        if (dr < cfg->dungeon_chance)      kind = STRUCTGEN_DUNGEON;
        else if (rr < cfg->ruin_chance)    kind = STRUCTGEN_RUIN;
        cx = chunk_x; cz = chunk_z;
    }

    if (kind == STRUCTGEN_NONE) {
        if (site) site->kind = STRUCTGEN_NONE;
        return STRUCTGEN_NONE;
    }

    // anchor at the chunk center column in world block coords.
    int wx = cx * CHUNK_SIZE_X + CHUNK_SIZE_X / 2;
    int wz = cz * CHUNK_SIZE_Z + CHUNK_SIZE_Z / 2;

    if (site) {
        site->kind     = kind;
        site->anchor_x = wx;
        site->anchor_z = wz;
        site->ground_y = 0;
        site->seed     = structgen_site_seed(kind, wx, wz, world_seed);
    }
    return kind;
}
