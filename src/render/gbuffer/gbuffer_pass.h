#ifndef RENDER_GBUFFER_PASS_H
#define RENDER_GBUFFER_PASS_H

#include "gbuffer.h"
#include "gbuffer_light.h"
#include "../texture.h"
#include "../../math/frustum.h"

// thin front-end that sequences the whole deferred frame so the renderer
// only has to deal with one object. it does NOT know about the world/chunk
// types — instead it takes a draw callback the caller fills with their own
// chunk iteration. keeps this module decoupled from world/.

// signature of the per-frame geometry emitter. called once, inside the
// geometry pass, with the gbuffer so the callback can push per-chunk model
// matrices via gbuffer_geometry_set_model and issue draws.
typedef void (*gbuffer_draw_fn)(gbuffer *g, void *user);

typedef struct {
    const texture     *atlas;     // block atlas for the geometry pass
    gbuffer_draw_fn     draw;      // emits opaque world geometry
    void               *user;      // passed through to draw
    const gbuffer_light_list *lights;
    const frustum      *frustum;   // for light culling, may be NULL
    vec3                eye;        // camera world pos
} gbuffer_frame;

// run geometry -> accumulate -> shade for one frame. the final shade blits
// to whatever framebuffer is bound on entry (caller decides: backbuffer or
// post-fx input). camera matrices must already be set via gbuffer_set_camera.
void gbuffer_pass_run(gbuffer *g, const gbuffer_frame *frame);

#endif
