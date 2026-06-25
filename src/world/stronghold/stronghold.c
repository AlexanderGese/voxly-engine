#include "stronghold.h"
#include "stronghold_rand.h"
#include "stronghold_room.h"
#include "stronghold_library.h"
#include "stronghold_portal.h"
#include "stronghold_corridor.h"
#include "stronghold_door.h"
#include <stddef.h>

uint32_t stronghold_site_seed(uint32_t world_seed, int region_x, int region_z) {
    // fold region coords into the world seed; the 0x57h salt is just "SH".
    uint32_t a = stronghold_hash2(region_x, region_z, world_seed ^ 0x57480000u);
    return stronghold_seed_mix(world_seed, a);
}

int stronghold_generate_ex(stronghold_buffer *out, stronghold_graph *graph,
                           const stronghold_site *site, const stronghold_config *cfg) {
    if (!out || !graph || !site || !cfg) return 0;
    if (cfg->max_rooms <= 0) return 0;

    // the portal room (deepest) anchors the dig depth. we root the first room
    // near the surface-minus-min_depth and let the walk burrow downward via
    // stair edges. all y here is world space.
    int oy = site->ground_y - cfg->min_depth;
    if (oy < 4) oy = 4;   // dont punch through bedrock

    stronghold_rng rng;
    stronghold_rng_seed(&rng, site->seed);

    // 1 + 2: build the abstract graph and type it.
    int rooms = stronghold_graph_grow(graph, cfg, site->anchor_x, oy, site->anchor_z, &rng);
    if (rooms == 0) return 0;
    stronghold_graph_assign_types(graph, cfg, &rng);

    int n = 0;

    // 3: generic room shells. must precede corridors so doorways punch cleanly.
    n += stronghold_room_carve_all(graph, out, &rng);

    // 4: the elaborate rooms. also before corridors, same reason.
    n += stronghold_library_carve_all(graph, out);
    n += stronghold_portal_build(graph, out, &rng);

    // 5: corridors. these mutate the graph (append door records) so the door
    // pass below has something to dress.
    n += stronghold_corridor_carve_all(graph, out);

    // 6: door dressing. cosmetic, runs last.
    n += stronghold_door_decorate_all(graph, out, &rng);

    // safety: if we blew past the voxel cap the caller asked for, we still
    // return what we have but it's a sign the config is too generous.
    if (cfg->max_voxels > 0 && out->count > cfg->max_voxels) {
        // truncate to the cap; partial strongholds beat OOM crashes.
        out->count = cfg->max_voxels;
    }
    return n;
}

int stronghold_generate(stronghold_buffer *out, const stronghold_site *site,
                        const stronghold_config *cfg) {
    // caller doesnt want the graph back, so we keep one on the stack. it's
    // a few KB of fixed arrays, fine for one-shot generation.
    stronghold_graph graph;
    return stronghold_generate_ex(out, &graph, site, cfg);
}
