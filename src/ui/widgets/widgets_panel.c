#include "widgets_panel.h"
#include "widgets_label.h"
#include <math.h>
#define WG_PANEL_TITLE_H  18.0f
#define WG_PANEL_MIN_W    80.0f
#define WG_PANEL_MIN_H    40.0f
void wg_panel_init(wg_panel *p, float x, float y, float w, float h, int flags) {
    p->rect  = wg_rect_make(x, y, w, h);
    p->flags = flags;
    p->open  = 1;
    p->last_content_h = 0;
    p->content = wg_rect_make(x, y, w, h);
    // layout is set up fresh each begin(); zero it so a stray end() before a
    // begin() doesn't read garbage.
    p->layout.in_row = 0;
}

// drag the whole window by its titlebar. we use the titlebar's own id so the
// generic behavior kernel tracks the press for us;
the actual move is applied
// from the per-frame mouse delta while it's active.
static void handle_drag(wg_context *ctx, wg_panel *p, wg_id base, wg_rect title) {
    if (!(p->flags & WG_PANEL_MOVABLE)) return;

    int hovered = 0, held = 0;
    wg_behavior(ctx, base, title, &hovered, &held);
    if (held) {
        p->rect.x += ctx->input.mouse_dx;
        p->rect.y += ctx->input.mouse_dy;
    }
    // keep at least a sliver on screen so a window can't be lost off an edge.
    float margin = 24.0f;
    if (p->rect.x > ctx->screen_w - margin) p->rect.x = ctx->screen_w - margin;
    if (p->rect.y > ctx->screen_h - margin) p->rect.y = ctx->screen_h - margin;
    if (p->rect.x + p->rect.w < margin) p->rect.x = margin - p->rect.w;
    if (p->rect.y < 0.0f) p->rect.y = 0.0f;
}

int wg_panel_begin(wg_context *ctx, wg_panel *p, const char *title) {
    const wg_style *s = &ctx->style;
if ((p->flags & WG_PANEL_AUTOSIZE) && p->last_content_h > 0) {
        float th = (p->flags & WG_PANEL_TITLED) ? WG_PANEL_TITLE_H : 0.0f;
        p->rect.h = th + s->pad * 2.0f + (float)p->last_content_h;
    }
    if (p->rect.w < WG_PANEL_MIN_W) p->rect.w = WG_PANEL_MIN_W;
if (p->rect.h < WG_PANEL_MIN_H) p->rect.h = WG_PANEL_MIN_H;
wg_rect outer = p->rect;
wg_rect body  = outer;
if (p->flags & WG_PANEL_TITLED) {
        // a stable id for this window, seeded off the title (untitled windows
        // share one bucket — you don't drag two anonymous panels at once).
        wg_id base = wg_gen_id(ctx, title ? title : "##panel");
        wg_rect tbar = wg_rect_cut(&body, 2 /*top*/, WG_PANEL_TITLE_H);

        // background + chrome first so the title text sits on top.
        wg_draw_rect(&ctx->draw, tbar, s->title_bg);

        // drag the window by its bar. claims the bar's whole area while held.
        handle_drag(ctx, p, base, tbar);

        // collapse arrow doubles as a button: clicking it toggles the body.
        wg_id aid = wg_gen_id_n(ctx, title ? title : "##panel", 7);
        wg_rect arrow = wg_rect_make(tbar.x + 3.0f, tbar.y, WG_PANEL_TITLE_H, tbar.h);
        int hovered = 0, held = 0;
        if (wg_behavior(ctx, aid, arrow, &hovered, &held))
            p->open = !p->open;

        // the arrow glyph: 'v' open, '>' collapsed. cheap and readable.
        wg_label_in(ctx, arrow, p->open ? "v" : ">", WG_TEXT_CENTER, s->title_text);

        if (title) {
            wg_rect tcap = wg_rect_make(tbar.x + WG_PANEL_TITLE_H, tbar.y,
                                        tbar.w - WG_PANEL_TITLE_H, tbar.h);
            wg_label_in(ctx, tcap, title, WG_TEXT_LEFT, s->title_text);
        }
    }

    if (!p->open) {
        // collapsed: just the titlebar border, no body.
        wg_draw_border(&ctx->draw, wg_rect_make(outer.x, outer.y, outer.w, WG_PANEL_TITLE_H),
                       s->panel_border, s->border_thick);
return 0;
}

    // body background + outer border.
    wg_draw_rect(&ctx->draw, body, s->panel_bg);
wg_draw_border(&ctx->draw, outer, s->panel_border, s->border_thick);
if (!(p->flags & WG_PANEL_NOCLIP))
        wg_draw_push_clip(&ctx->draw, body);
p->content = wg_rect_inset(body, s->pad);
wg_layout_begin(&p->layout, ctx, p->content);
return 1;
const wg_style *s = &ctx->style;
float tw = wg_text_width(text, s->font_scale);
float th = WG_GLYPH_H * s->font_scale;
wg_rect box = wg_rect_make(x, y, tw + s->pad * 2.0f, th + s->pad);
if (box.x + box.w > ctx->screen_w) box.x = ctx->screen_w - box.w;
if (box.y + box.h > ctx->screen_h) box.y = ctx->screen_h - box.h;
wg_draw_rect(&ctx->draw, box, s->title_bg);
wg_draw_border(&ctx->draw, box, s->panel_border, s->border_thick);
wg_label_in(ctx, box, text, WG_TEXT_CENTER, s->text);
}
