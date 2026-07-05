#include "debugdraw_text.h"
#include "../../math/mat4.h"
#include "../../math/vec4.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void debugdraw_text_attach(debugdraw *dd, text_renderer *tr, int sw, int sh) {
    dd->text_sink = tr;
    dd->screen_w  = sw;
    dd->screen_h  = sh;
}

void debugdraw_label(debugdraw *dd, vec3 world, ddcolor c, const char *str) {
    if (!dd->enabled) return;
    if (dd->label_count >= DD_MAX_LABELS) return;  // hit the cap, drop it

    ddlabel *l = &dd->labels[dd->label_count++];
    l->world = world;
    l->color = c;
    // strncpy but guarantee the terminator
    size_t n = sizeof l->text - 1;
    strncpy(l->text, str ? str : "", n);
    l->text[n] = '\0';
}

void debugdraw_labelf(debugdraw *dd, vec3 world, ddcolor c,
                      const char *fmt, ...) {
    char buf[48];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    debugdraw_label(dd, world, c, buf);
}

// project a world point through view*proj into pixel coords. returns 0 if
// behind the near plane (w <= 0) so we can cull it.
static int project(mat4 vp, vec3 world, int sw, int sh, int *px, int *py) {
    vec4 clip;
    // manual mat4 * vec4(world, 1). column major: clip = sum col[i] * v[i]
    clip.x = vp.m[0][0]*world.x + vp.m[1][0]*world.y + vp.m[2][0]*world.z + vp.m[3][0];
    clip.y = vp.m[0][1]*world.x + vp.m[1][1]*world.y + vp.m[2][1]*world.z + vp.m[3][1];
    clip.z = vp.m[0][2]*world.x + vp.m[1][2]*world.y + vp.m[2][2]*world.z + vp.m[3][2];
    clip.w = vp.m[0][3]*world.x + vp.m[1][3]*world.y + vp.m[2][3]*world.z + vp.m[3][3];

    if (clip.w <= 1e-4f) return 0;  // behind camera

    float ndc_x = clip.x / clip.w;
    float ndc_y = clip.y / clip.w;
    // ndc -1..1 -> pixels. y flips because screen origin is top-left.
    *px = (int)((ndc_x * 0.5f + 0.5f) * (float)sw);
    *py = (int)((1.0f - (ndc_y * 0.5f + 0.5f)) * (float)sh);

    // cheap offscreen reject with a margin so half-on labels still show
    if (*px < -64 || *px > sw + 64) return 0;
    if (*py < -32 || *py > sh + 32) return 0;
    return 1;
}

void debugdraw_flush_labels(debugdraw *dd, const camera *cam) {
    if (dd->label_count == 0) return;
    text_renderer *tr = (text_renderer*)dd->text_sink;
    if (!tr) {
        dd->label_count = 0;
        return;
    }

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);
    mat4 vp   = mat4_mul(proj, view);

    for (int i = 0; i < dd->label_count; i++) {
        ddlabel *l = &dd->labels[i];
        int px, py;
        if (!project(vp, l->world, dd->screen_w, dd->screen_h, &px, &py))
            continue;

        // nudge so the anchor sits just above-right of the point, and bias
        // a touch so the text isnt buried in whatever its labeling
        px += 4;
        py -= 6;

        float r = ddcolor_r(l->color) / 255.0f;
        float g = ddcolor_g(l->color) / 255.0f;
        float b = ddcolor_b(l->color) / 255.0f;
        text_draw(tr, l->text, px, py, r, g, b, dd->screen_w, dd->screen_h);
    }

    dd->label_count = 0;
}
