#include "widgets_context.h"

#include <string.h>

// fnv-1a, same hash the text label cache uses. 32-bit is plenty for widget ids.
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

    // remember last frame's hot so widgets can detect first-hover, then clear
    // the candidate — widgets re-claim it during traversal.
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
    if (id == 0) id = 1;   // 0 is our "nothing" sentinel, never hand it out
    return id;
}

wg_id wg_gen_id_n(wg_context *ctx, const char *label, int salt) {
    wg_id base = wg_gen_id(ctx, label);
    // mix the salt in so e.g. list rows sharing a label still differ.
    base ^= (uint32_t)salt * 2654435761u;
    base *= 16777619u;
    if (base == 0) base = 1;
    return base;
}

void wg_push_id(wg_context *ctx, wg_id id) {
    if (ctx->id_top >= WG_ID_STACK) return;   // silently cap, see header note
    ctx->id_stack[ctx->id_top++] = id;
}

void wg_pop_id(wg_context *ctx) {
    if (ctx->id_top > 0) ctx->id_top--;
}

int wg_is_hot(const wg_context *ctx, wg_id id) {
    return id != 0 && ctx->hot == id;
}

int wg_is_active(const wg_context *ctx, wg_id id) {
    return id != 0 && ctx->active == id;
}

int wg_behavior(wg_context *ctx, wg_id id, wg_rect r,
                int *out_hovered, int *out_held) {
    wg_input *in = &ctx->input;
    int over = wg_input_over(in, r);
    int clicked = 0;

    // a widget can only become hot if nothing is being dragged, or it's the
    // thing being dragged. that stops the cursor "snagging" on other widgets
    // mid-drag, which feels awful.
    if (over && (ctx->active == 0 || ctx->active == id)) {
        ctx->hot = id;
        ctx->want_mouse = 1;
    }

    if (ctx->active == id) {
        // we own the press. on release decide if it counts as a click.
        if (wg_input_mouse_released(in, WG_MOUSE_LEFT)) {
            if (ctx->hot == id) clicked = 1;
            // active is cleared centrally in wg_end, but drop the focus-grab here
        }
    } else if (ctx->hot == id) {
        // not active yet; a fresh press claims us.
        if (wg_input_mouse_pressed(in, WG_MOUSE_LEFT)) {
            ctx->active = id;
            ctx->focus = id;
            // snapshot the grab anchor for drag widgets to lean on.
            ctx->active_mouse0 = in->mouse_x;
            ctx->active_v0 = 0.0f;
        }
    }

    if (out_hovered) *out_hovered = (ctx->hot == id);
    if (out_held) *out_held = (ctx->active == id);
    return clicked;
}
