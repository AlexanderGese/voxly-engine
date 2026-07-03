#ifndef RENDER_CULL_H
#define RENDER_CULL_H

#include "../../math/mat4.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

#include "frustum_cull.h"
#include "lod.h"
#include "occlusion.h"
#include "draw_list.h"

// top-level cull subsystem. one struct holds everything, you init it once
// and call cull_run every frame with the camera matrices and the world.
// it spits out an ordered list of chunks to draw, already frustum + lod +
// occlusion filtered, opaque sorted near->far.
//
// the renderer is expected to walk cull_visible() and draw them. nothing
// in here touches GL, it's pure cpu bookkeeping.

typedef struct {
    cull_frustum    frustum;
    cull_lod_config lod;
    cull_occluder   occ;
    cull_draw_list  list;

    // toggles. mirror the config.h debug switches.
    int  do_frustum;
    int  do_occlusion;
    int  do_lod;

    float render_dist_chunks;   // hard distance cutoff, chunk units
    int   prev_lod_valid;       // whether items carry usable prev lod
} cull_ctx;

void cull_init(cull_ctx *cc);
void cull_shutdown(cull_ctx *cc);

// the whole pass. vp = proj*view. cam_pos for distance + occlusion origin.
// returns number of chunks that survived (will be drawn).
int  cull_run(cull_ctx *cc, world *w, mat4 vp, vec3 cam_pos);

// the surviving, ordered draw list. valid until the next cull_run.
const cull_item *cull_visible(const cull_ctx *cc, int *count_out);

// stats from the last pass, for the F3 overlay.
const cull_stats *cull_last_stats(const cull_ctx *cc);

#endif
