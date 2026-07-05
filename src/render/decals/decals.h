#ifndef RENDER_DECALS_H
#define RENDER_DECALS_H

#include "decals_types.h"
#include "decals_pool.h"
#include "decals_atlas.h"
#include "decals_cull.h"
#include "decals_pass.h"
#include "../gl.h"
#include "../../math/mat4.h"

// top-level deferred decal system. owns the pool, the atlas, the per-frame cull
// result and the gl pass. the renderer drives it:
//
// decals_init(&dec)                       once, after gl is up
// ... gameplay spawns decals via decals_stamp / decals_spawn ...
// decals_update(&dec, dt)                 advance fades, reclaim dead slots
// decals_set_camera(&dec, vp, inv_vp, cam, w, h)
// ... renderer binds the g-buffer with blending ...
// decals_render(&dec, depth_tex, normal_tex)
//
// the system never touches the framebuffer binding itself — the renderer owns
// that and just hands us the two g-buffer textures we sample.

typedef struct {
    decals_pool        pool;
    decals_atlas       atlas;
    decals_cull_result vis;     // last gather, reused each frame
    decals_pass        pass;

    mat4  view_proj, inv_view_proj;
    vec3  cam_pos;
    float max_dist;             // transient-decal distance cutoff
    int   ready;                // 0 if gl init failed; render() then no-ops
} decals_system;

// init everything. uploads no atlas texture — call decals_set_atlas_texture
// once the decal page is loaded. returns 1 on success.
int  decals_init(decals_system *s);
void decals_shutdown(decals_system *s);

// hand over the loaded atlas texture id (created by the renderer's texture
// loader). registers the default region set against it.
void decals_set_atlas_texture(decals_system *s, glid atlas_tex);

// advance fades, reclaim dead slots. call once per frame with the frame dt.
void decals_update(decals_system *s, float dt);

// per-frame camera state for cull + reconstruction.
void decals_set_camera(decals_system *s, mat4 view_proj, mat4 inv_view_proj,
                       vec3 cam_pos, int screen_w, int screen_h);
void decals_set_max_dist(decals_system *s, float d);

// cull + draw. samples the two g-buffer textures, stamps into the bound fbo.
// returns the number of decals drawn.
int  decals_render(decals_system *s, glid depth_tex, glid gnormal_tex);

// --- spawning ---------------------------------------------------------------

// raw spawn from a fully-built descriptor. resolves the named region; returns
// the handle (or invalid if the region is unknown / descriptor degenerate).
decals_handle decals_spawn_named(decals_system *s, const char *region_name,
                                 decals_spawn_desc *desc);

// the common case: stamp a decal of `region_name` onto a surface. `hit` is the
// surface point, `surf_normal` the surface normal (the projector forward is set
// to -normal so the box looks at the wall), `size` is the full footprint
// width/height, `depth` how far the box reaches into the surface. picks an up
// hint, builds the projector, fills sane fade defaults, spawns. rotation
// randomises the stamp orientation a touch so repeated stamps dont tile.
decals_handle decals_stamp(decals_system *s, const char *region_name,
                           vec3 hit, vec3 surf_normal,
                           float size, float depth, float rotation);

// kill a decal early (eases it out).
void decals_remove(decals_system *s, decals_handle h);

int  decals_count(const decals_system *s);

#endif
