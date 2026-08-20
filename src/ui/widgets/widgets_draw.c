#include "widgets_draw.h"

#include "../../util/darray.h"
#include <string.h>
#include <math.h>

void wg_draw_init(wg_draw_list *dl) {
    dl->cmds = NULL;
    dl->clips = NULL;
    dl->cur_clip = -1;
}

void wg_draw_free(wg_draw_list *dl) {
    darr_free(dl->cmds);
    darr_free(dl->clips);
    dl->cur_clip = -1;
}

void wg_draw_reset(wg_draw_list *dl) {
    darr_clear(dl->cmds);
    darr_clear(dl->clips);
    dl->cur_clip = -1;
}

// intersect two rects. if they don't overlap the result is a zero-size rect at
// a's origin, which the host's scissor will treat as "draw nothing".
static wg_rect rect_intersect(wg_rect a, wg_rect b) {
    float x0 = a.x > b.x ? a.x : b.x;
    float y0 = a.y > b.y ? a.y : b.y;
    float x1 = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    float y1 = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    if (x1 < x0) x1 = x0;
    if (y1 < y0) y1 = y0;
    return wg_rect_make(x0, y0, x1 - x0, y1 - y0);
}

void wg_draw_push_clip(wg_draw_list *dl, wg_rect r) {
    wg_rect eff = r;
    if (dl->cur_clip >= 0)
        eff = rect_intersect(dl->clips[dl->cur_clip], r);
    darr_push(dl->clips, eff);
    dl->cur_clip = (int)darr_len(dl->clips) - 1;
}

void wg_draw_pop_clip(wg_draw_list *dl) {
    if (dl->cur_clip < 0) return;
    // we don't shrink the array (cheaper to keep the slot) — just walk the top
    // pointer back down. find the previous live index by length bookkeeping.
    size_t n = darr_len(dl->clips);
    if (n > 0) darr_hdr(dl->clips)->len = n - 1;
    dl->cur_clip = (int)darr_len(dl->clips) - 1;
}

static wg_cmd *emit(wg_draw_list *dl, wg_cmd_kind k) {
    wg_cmd c;
    memset(&c, 0, sizeof c);
    c.kind = k;
    c.clip = dl->cur_clip;
    c.thickness = 1.0f;
    c.text_scale = 1.0f;
    darr_push(dl->cmds, c);
    return &darr_last(dl->cmds);
}

void wg_draw_rect(wg_draw_list *dl, wg_rect r, wg_rgba c) {
    // fully transparent fills are a no-op; saves the host a pointless draw.
    if ((c >> 24) == 0) return;
    wg_cmd *cmd = emit(dl, WG_CMD_RECT);
    cmd->rect = r;
    cmd->color = c;
}

void wg_draw_border(wg_draw_list *dl, wg_rect r, wg_rgba c, float thickness) {
    if ((c >> 24) == 0) return;
    wg_cmd *cmd = emit(dl, WG_CMD_BORDER);
    cmd->rect = r;
    cmd->color = c;
    cmd->thickness = thickness;
}

void wg_draw_line(wg_draw_list *dl, float x0, float y0, float x1, float y1,
                  wg_rgba c, float thickness) {
    if ((c >> 24) == 0) return;
    wg_cmd *cmd = emit(dl, WG_CMD_LINE);
    // stash the span in rect: (x,y) start, (w,h) end. the host reads it as two
    // points, not as a box.
    cmd->rect = wg_rect_make(x0, y0, x1, y1);
    cmd->color = c;
    cmd->thickness = thickness;
}

void wg_draw_text(wg_draw_list *dl, float x, float y, const char *s,
                  float scale, wg_rgba c) {
    if (!s || !s[0]) return;
    wg_cmd *cmd = emit(dl, WG_CMD_TEXT);
    cmd->rect = wg_rect_make(x, y, 0, 0);
    cmd->color = c;
    cmd->text_scale = scale;
    // copy with truncation, always nul-terminated.
    size_t i = 0;
    for (; s[i] && i < WG_TEXT_MAX - 1; i++)
        cmd->text[i] = s[i];
    cmd->text[i] = 0;
}

void wg_draw_panel(wg_draw_list *dl, wg_rect r, wg_rgba fill, wg_rgba border) {
    wg_draw_rect(dl, r, fill);
    wg_draw_border(dl, r, border, 1.0f);
}

int wg_draw_count(const wg_draw_list *dl) {
    return (int)darr_len(dl->cmds);
}
