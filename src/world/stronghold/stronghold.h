#ifndef WORLD_STRONGHOLD_H
#define WORLD_STRONGHOLD_H

#include "stronghold_types.h"
#include "stronghold_buffer.h"
#include "stronghold_graph.h"

// stronghold generator driver. one call takes a site (anchor + surface y +
// seed) and a config, grows a room graph, carves it into a voxel buffer, and
// hands the buffer back for the worldgen stamper to splat into chunks.
//
// pipeline, in order (order matters, see the comments):
// 1. grow the room graph         (abstract boxes, no voxels)
// 2. assign room types           (one portal, libraries, prisons)
// 3. carve generic room shells   (brick + hollow + lighting)
// 4. carve libraries + portal    (their own elaborate builders)
// 5. carve corridors             (tunnels + punch doorways in shells)
// 6. decorate doors              (dress the punched holes)
//
// the whole thing is deterministic on (site.seed): same seed, same maze.

// full build. fills out with the stronghold's voxels. returns voxels emitted,
// or 0 if nothing was generated (e.g. degenerate config).
int stronghold_generate(stronghold_buffer *out, const stronghold_site *site,
                        const stronghold_config *cfg);

// build into a caller-provided graph too, so callers that want the room layout
// (for map markers, locate commands, etc.) can keep it. same pipeline.
int stronghold_generate_ex(stronghold_buffer *out, stronghold_graph *graph,
                           const stronghold_site *site, const stronghold_config *cfg);

// derive the per-site seed from a world seed + region coords. keeps stronghold
// placement independent of terrain noise.
uint32_t stronghold_site_seed(uint32_t world_seed, int region_x, int region_z);

#endif
