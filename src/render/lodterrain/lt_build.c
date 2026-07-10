#include "lt_build.h"

#include "lt_grid.h"
#include "lt_sample.h"
#include "lt_surface.h"
#include "lt_skirt.h"
#include <stddef.h>

int lt_build_chunk(lt_mesh *out, const lt_source *src, int level,
                   const lt_neighbor_levels *nb) {
    lt_mesh_reset(out);

    lt_grid g;
    if (!lt_grid_init(&g, level)) {
        // oom. leave the mesh empty-but-valid; the manager will retry next time
        // it's marked dirty. better an empty far chunk than a crash.
        return 0;
    }

    int solid = lt_sample_grid(&g, src);
    if (solid == 0) {
        // nothing solid in this column at this resolution — air pocket or above
        // the world. an empty mesh is a perfectly good answer.
        lt_grid_free(&g);
        return 1;
    }

    lt_surface_build(out, &g, src);

    if (nb) {
        lt_seam_mask mask = lt_seam_compute(g.level, nb);
        out->skirt_quads += lt_skirt_build(out, &g, src, mask);
    }

    lt_grid_free(&g);
    return 1;
}

int lt_build_chunk_simple(lt_mesh *out, const lt_source *src, int level) {
    // neighbours all at our level -> no seams -> pass NULL through.
    return lt_build_chunk(out, src, level, NULL);
}
