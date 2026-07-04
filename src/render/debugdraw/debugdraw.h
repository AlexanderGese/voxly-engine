#ifndef RENDER_DEBUGDRAW_H
#define RENDER_DEBUGDRAW_H

#include "../camera.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "debugdraw_color.h"
#include "debugdraw_vertex.h"
#include "debugdraw_gl.h"

// immediate-mode debug drawing. you queue primitives during the frame and
// flush them all at the end. supports depth-tested and overlay passes, plus
// timed entries that persist for N seconds (see debugdraw_timed).
//
// usage:
// debugdraw_init(&dd);
// ... each frame ...
// debugdraw_new_frame(&dd, dt);
// debugdraw_line(&dd, a, b, DDCOLOR_RED);
// debugdraw_box(&dd, my_aabb, DDCOLOR_GREEN);
// debugdraw_flush(&dd, cam);
//
// not thread safe. one of these, owned by the renderer.

// max screen-space text labels per frame. labels project to 2d and get
// handed to the hud text renderer, so theres a hard cap.
#define DD_MAX_LABELS  256

typedef struct {
    char  text[48];
    vec3  world;       // anchor in world space
    ddcolor color;
} ddlabel;

// a queued line/point vertex plus which bucket it belongs to. we sort by
// bucket at flush so each gl draw call is one state set.
typedef enum {
    DD_BUCKET_LINE_DEPTH = 0,   // depth-tested lines
    DD_BUCKET_LINE_OVER,        // overlay lines (no depth)
    DD_BUCKET_POINT_DEPTH,
    DD_BUCKET_POINT_OVER,
    DD_BUCKET_COUNT
} ddbucket;

struct ddtimed;  // fwd, lives in debugdraw_timed.h

typedef struct debugdraw {
    ddgl    gl;

    // per-bucket vertex queues (darrays of ddvert)
    ddvert *buckets[DD_BUCKET_COUNT];

    // merged upload scratch, rebuilt each flush
    ddvert *scratch;

    ddlabel labels[DD_MAX_LABELS];
    int     label_count;

    // hud text sink for labels, attached via debugdraw_text_attach.
    // opaque here (void*) so the core doesnt have to pull in text.h; the
    // text module casts it back to text_renderer*.
    void   *text_sink;
    int     screen_w, screen_h;

    // timed entries persist across frames, see debugdraw_timed.c
    struct ddtimed *timed;   // darray
    float   now;             // accumulated time, seconds

    int     enabled;
    int     depth_test;      // default depth state for new lines
    float   line_width;
    float   point_size;

    // category filtering, see debugdraw_category.h
    uint32_t cat_mask;       // which categories are allowed to draw
    uint32_t cat_stack[8];   // push/pop scope of the active category bit
    int      cat_depth;

    // stats, handy for the hud
    int     stat_verts;
    int     stat_draws;
} debugdraw;

int  debugdraw_init(debugdraw *dd);
void debugdraw_destroy(debugdraw *dd);

void debugdraw_set_enabled(debugdraw *dd, int on);
int  debugdraw_enabled(const debugdraw *dd);

// call at the start of a frame. clears the per-frame queues and advances
// the clock so timed entries can expire. dt in seconds.
void debugdraw_new_frame(debugdraw *dd, float dt);

// toggle depth testing for subsequent line/point calls this frame
void debugdraw_depth(debugdraw *dd, int on);

// --- core primitives ---
void debugdraw_line(debugdraw *dd, vec3 a, vec3 b, ddcolor c);
void debugdraw_line2(debugdraw *dd, vec3 a, vec3 b, ddcolor ca, ddcolor cb);
void debugdraw_point(debugdraw *dd, vec3 p, ddcolor c);
void debugdraw_ray(debugdraw *dd, vec3 origin, vec3 dir, float len, ddcolor c);

// quick aabb wireframe (12 edges). shapes module has fancier stuff.
void debugdraw_box(debugdraw *dd, aabb a, ddcolor c);

// project labels, upload geometry, issue draws. needs the active camera
// for the view*proj and for label projection.
void debugdraw_flush(debugdraw *dd, const camera *cam);

// internal: append one line segment into the right bucket. used by the
// shapes/timed modules too. honors the current depth state.
void debugdraw_emit_line(debugdraw *dd, vec3 a, vec3 b, ddcolor ca, ddcolor cb);
void debugdraw_emit_point(debugdraw *dd, vec3 p, ddcolor c);

// defined in debugdraw_text.c. forward-declared here so flush can call it
// without the core including text.h. no-op if no text sink is attached.
void debugdraw_flush_labels(debugdraw *dd, const camera *cam);

#endif
