#ifndef RENDER_LODTERRAIN_LT_TERRAIN_H
#define RENDER_LODTERRAIN_LT_TERRAIN_H

#include "lt_types.h"
#include "lt_cache.h"
#include "lt_select.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../math/frustum.h"
#include "../../world/world.h"
#include "../gl.h"

// the public face of the terrain lod subsystem. wires the selector, cache,
// builder and gpu uploader into a single per-frame update + draw. the renderer
// uses this for the far rings while the regular per-block mesher handles the
// near chunks; somewhere out around the level-1 ring we hand off and the player
// is none the wiser (that's the dream, anyway).

// rolling stats for the F3 overlay. reset at the top of each update.
typedef struct {
    int  chunks_considered;
    int  builds_this_frame;   // capped by the budget
    int  builds_deferred;     // wanted a build but ran out of budget
    int  drawn;
    long tris_drawn;
} lt_terrain_stats;

typedef struct {
    lt_cache         cache;
    lt_select_config select;
    int              build_budget;   // max (re)builds per update, throttling
    int              enabled;
    lt_terrain_stats stats;

    // scratch mesh reused across builds so we don't malloc/free per chunk.
    lt_mesh          scratch;
} lt_terrain;

// stand the manager up. build_budget of ~4 keeps frame spikes small; pass 0 for
// a sane default.
void lt_terrain_init(lt_terrain *t, int build_budget);
void lt_terrain_destroy(lt_terrain *t);

// per-frame tick: re-select levels for every loaded chunk, rebuild up to the
// budget, evict caches for chunks that streamed out. `cam_pos` drives selection,
// `dt` (seconds) advances the level-transition morphs.
void lt_terrain_update(lt_terrain *t, world *w, vec3 cam_pos, float dt);

// draw the far terrain with the given shader already bound. `vp` is the
// view*proj used for frustum rejection so we skip uploading-but-offscreen rings.
void lt_terrain_render(lt_terrain *t, glid shader, mat4 vp);

// poke a chunk's cached mesh as stale (e.g. the player edited a far block). safe
// if the chunk isn't cached.
void lt_terrain_invalidate(lt_terrain *t, int cx, int cz);

#endif
