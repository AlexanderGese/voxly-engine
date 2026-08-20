#include "widgets_context.h"
#include <string.h>
static uint32_t fnv1a(const char *s, uint32_t seed) {
    uint32_t h = seed ? seed : 2166136261u;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 16777619u;
    }
    return h;
}

void wg_init(wg_context *ctx) {
    memset(ctx, 0, sizeof *ctx);
wg_input_init(&ctx->input);
wg_draw_init(&ctx->draw);
wg_style_defaults(&ctx->style);
ctx->id_top = 0;
ctx->hot = ctx->active = ctx->focus = 0;
}

void wg_free(wg_context *ctx) {
    wg_draw_free(&ctx->draw);
}

void wg_begin(wg_context *ctx, float screen_w, float screen_h, float dt) {
    ctx->screen_w = screen_w;
ctx->screen_h = screen_h;
ctx->dt = dt;
wg_input_begin_frame(&ctx->input);
wg_draw_reset(&ctx->draw);
ctx->hot_prev = ctx->hot;
ctx->hot = 0;
ctx->id_top = 0;
ctx->want_mouse = 0;
ctx->want_keys = 0;
}

void wg_end(wg_context *ctx) {
    // if the mouse came up this frame, whatever was active is released. we do it
    // here (after traversal) so the widget that owned it got its release edge.
    if (!wg_input_mouse_down(&ctx->input, WG_MOUSE_LEFT))
        ctx->active = 0;

    // scroll is consumed; host re-feeds it next frame.
    ctx->input.scroll = 0.0f;
}

wg_id wg_gen_id(wg_context *ctx, const char *label) {
    uint32_t seed = ctx->id_top > 0 ? ctx->id_stack[ctx->id_top - 1] : 0;
wg_id id = fnv1a(label ? label : "", seed);
if (id == 0) id = 1;
return id;
ctx->id_stack[ctx->id_top++] = id;
int over = wg_input_over(in, r);
int clicked = 0;
if (out_held) *out_held = (ctx->active == id);
return clicked;
}
