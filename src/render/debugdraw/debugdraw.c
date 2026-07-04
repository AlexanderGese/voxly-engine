#include "debugdraw.h"
#include "debugdraw_timed.h"
#include "../../util/darray.h"
#include "../../util/log.h"
#include "../../math/mat4.h"

#include <string.h>

int debugdraw_init(debugdraw *dd) {
    memset(dd, 0, sizeof *dd);
    if (!ddgl_init(&dd->gl)) {
        LOGE("debugdraw_init: gl backend failed");
        return 0;
    }
    for (int i = 0; i < DD_BUCKET_COUNT; i++)
        dd->buckets[i] = NULL;
    dd->scratch     = NULL;
    dd->timed       = NULL;
    dd->enabled     = 1;
    dd->depth_test  = 1;
    dd->line_width  = 1.5f;
    dd->point_size  = 6.0f;
    dd->now         = 0.0f;
    dd->cat_mask    = 0xffffffffu;   // everything on by default
    dd->cat_stack[0] = 1u << 0;      // DD_CAT_GENERAL at the bottom
    dd->cat_depth   = 0;
    LOGI("debugdraw ready");
    return 1;
}

void debugdraw_destroy(debugdraw *dd) {
    for (int i = 0; i < DD_BUCKET_COUNT; i++)
        darr_free(dd->buckets[i]);
    darr_free(dd->scratch);
    darr_free(dd->timed);
    ddgl_destroy(&dd->gl);
}

void debugdraw_set_enabled(debugdraw *dd, int on) { dd->enabled = !!on; }
int  debugdraw_enabled(const debugdraw *dd)        { return dd->enabled; }

void debugdraw_new_frame(debugdraw *dd, float dt) {
    dd->now += dt;
    for (int i = 0; i < DD_BUCKET_COUNT; i++)
        darr_clear(dd->buckets[i]);
    dd->label_count = 0;
    dd->depth_test  = 1;
    dd->stat_verts  = 0;
    dd->stat_draws  = 0;
}

void debugdraw_depth(debugdraw *dd, int on) { dd->depth_test = !!on; }

// pick the line bucket given the current depth state
static ddbucket line_bucket(const debugdraw *dd) {
    return dd->depth_test ? DD_BUCKET_LINE_DEPTH : DD_BUCKET_LINE_OVER;
}
static ddbucket point_bucket(const debugdraw *dd) {
    return dd->depth_test ? DD_BUCKET_POINT_DEPTH : DD_BUCKET_POINT_OVER;
}

// current category visible under the mask? mirrors the logic in
// debugdraw_category.c but inlined here so the hot emit path stays cheap.
static int cat_visible(const debugdraw *dd) {
    uint32_t cur = dd->cat_stack[dd->cat_depth];
    return (dd->cat_mask & cur) != 0;
}

void debugdraw_emit_line(debugdraw *dd, vec3 a, vec3 b, ddcolor ca, ddcolor cb) {
    if (!dd->enabled || !cat_visible(dd)) return;
    ddbucket bk = line_bucket(dd);
    darr_push(dd->buckets[bk], ddvert_make(a, ca));
    darr_push(dd->buckets[bk], ddvert_make(b, cb));
}

void debugdraw_emit_point(debugdraw *dd, vec3 p, ddcolor c) {
    if (!dd->enabled || !cat_visible(dd)) return;
    ddbucket bk = point_bucket(dd);
    darr_push(dd->buckets[bk], ddvert_make(p, c));
}

void debugdraw_line(debugdraw *dd, vec3 a, vec3 b, ddcolor c) {
    debugdraw_emit_line(dd, a, b, c, c);
}

void debugdraw_line2(debugdraw *dd, vec3 a, vec3 b, ddcolor ca, ddcolor cb) {
    debugdraw_emit_line(dd, a, b, ca, cb);
}

void debugdraw_point(debugdraw *dd, vec3 p, ddcolor c) {
    debugdraw_emit_point(dd, p, c);
}

void debugdraw_ray(debugdraw *dd, vec3 origin, vec3 dir, float len, ddcolor c) {
    vec3 end = vec3_add(origin, vec3_scale(vec3_normalize(dir), len));
    debugdraw_emit_line(dd, origin, end, c, c);
}

void debugdraw_box(debugdraw *dd, aabb a, ddcolor c) {
    vec3 mn = a.min, mx = a.max;
    // 8 corners
    vec3 v[8] = {
        { mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z },
        { mx.x, mn.y, mx.z }, { mn.x, mn.y, mx.z },
        { mn.x, mx.y, mn.z }, { mx.x, mx.y, mn.z },
        { mx.x, mx.y, mx.z }, { mn.x, mx.y, mx.z },
    };
    // bottom ring, top ring, 4 verticals
    static const int e[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7},
    };
    for (int i = 0; i < 12; i++)
        debugdraw_emit_line(dd, v[e[i][0]], v[e[i][1]], c, c);
}

// flatten all four buckets into the scratch darray in bucket order and
// remember the (first,count) span of each so flush can issue one draw per
// bucket. returns total verts written.
static int build_scratch(debugdraw *dd, int spans[DD_BUCKET_COUNT][2]) {
    darr_clear(dd->scratch);
    int cursor = 0;
    for (int b = 0; b < DD_BUCKET_COUNT; b++) {
        int n = (int)darr_len(dd->buckets[b]);
        spans[b][0] = cursor;
        spans[b][1] = n;
        for (int i = 0; i < n; i++)
            darr_push(dd->scratch, dd->buckets[b][i]);
        cursor += n;
    }
    return cursor;
}

void debugdraw_flush(debugdraw *dd, const camera *cam) {
    if (!dd->enabled) return;

    // re-emit persistent timed entries first so they land in the buckets
    debugdraw_timed_tick(dd);

    int spans[DD_BUCKET_COUNT][2];
    int total = build_scratch(dd, spans);
    if (total == 0) {
        // still let labels through even with no geometry
        debugdraw_flush_labels(dd, cam);
        return;
    }

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);
    mat4 vp   = mat4_mul(proj, view);

    ddgl_upload(&dd->gl, dd->scratch, total);
    ddgl_begin(&dd->gl, mat4_data(&vp));

    // order matters: depth-tested first, overlays last so they win
    if (spans[DD_BUCKET_LINE_DEPTH][1]) {
        ddgl_draw_range(&dd->gl, GL_LINES,
                        spans[DD_BUCKET_LINE_DEPTH][0],
                        spans[DD_BUCKET_LINE_DEPTH][1],
                        1, dd->line_width, dd->point_size);
        dd->stat_draws++;
    }
    if (spans[DD_BUCKET_POINT_DEPTH][1]) {
        ddgl_draw_range(&dd->gl, GL_POINTS,
                        spans[DD_BUCKET_POINT_DEPTH][0],
                        spans[DD_BUCKET_POINT_DEPTH][1],
                        1, dd->line_width, dd->point_size);
        dd->stat_draws++;
    }
    if (spans[DD_BUCKET_LINE_OVER][1]) {
        ddgl_draw_range(&dd->gl, GL_LINES,
                        spans[DD_BUCKET_LINE_OVER][0],
                        spans[DD_BUCKET_LINE_OVER][1],
                        0, dd->line_width, dd->point_size);
        dd->stat_draws++;
    }
    if (spans[DD_BUCKET_POINT_OVER][1]) {
        ddgl_draw_range(&dd->gl, GL_POINTS,
                        spans[DD_BUCKET_POINT_OVER][0],
                        spans[DD_BUCKET_POINT_OVER][1],
                        0, dd->line_width, dd->point_size);
        dd->stat_draws++;
    }

    ddgl_end(&dd->gl);
    dd->stat_verts = total;

    // labels go on top, in screen space
    debugdraw_flush_labels(dd, cam);
}
