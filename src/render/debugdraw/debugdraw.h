#ifndef RENDER_DEBUGDRAW_H
#define RENDER_DEBUGDRAW_H
#include "../camera.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "debugdraw_color.h"
#include "debugdraw_vertex.h"
#include "debugdraw_gl.h"
#define DD_MAX_LABELS  256
typedef struct {
    char  text[48];
    vec3  world;       // anchor in world space
    ddcolor color;
} ddlabel;
typedef enum {
    DD_BUCKET_LINE_DEPTH = 0,   // depth-tested lines
    DD_BUCKET_LINE_OVER,        // overlay lines (no depth)
    DD_BUCKET_POINT_DEPTH,
    DD_BUCKET_POINT_OVER,
    DD_BUCKET_COUNT
} ddbucket;
struct ddtimed;
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
void debugdraw_new_frame(debugdraw *dd, float dt);
void debugdraw_depth(debugdraw *dd, int on);
#endif
