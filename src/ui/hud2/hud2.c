#include "hud2.h"
#include "hud2_layout.h"
#include "hud2_hurt.h"
#include "hud2_blockicon.h"
#include "hud2_color.h"
#include "hud2_anim.h"
#include <stddef.h>

void hud2_init(hud2 *h, glid prog, text_renderer *text) {
    h->prog = prog;
    h->text = text;
    h->sw   = WIN_WIDTH;
    h->sh   = WIN_HEIGHT;

    hud2_batch_init(&h->batch, prog);
    hud2_hotbar_init(&h->hotbar);
    hud2_bars_init(&h->bars);
    hud2_crosshair_init(&h->crosshair);
    hud2_toast_init(&h->toasts);
    hud2_vignette_init(&h->vignette);
    hud2_xpbar_init(&h->xpbar);
    hud2_cooldown_init(&h->cooldown);

    h->swing_recover = 0.5f;   // default; gameplay can override per-tool
    h->inited = 1;
}

void hud2_destroy(hud2 *h) {
    if (!h->inited) return;
    hud2_batch_destroy(&h->batch);
    h->inited = 0;
}

void hud2_update(hud2 *h, const inventory *inv, const survival *s,
                 int targeting, int sw, int sh, float dt) {
    h->sw = sw;
    h->sh = sh;

    hud2_hotbar_update(&h->hotbar, inv, sw, sh, dt);
    hud2_bars_update(&h->bars, s, dt);
    hud2_crosshair_update(&h->crosshair, targeting, dt);
    hud2_toast_update(&h->toasts, sw, sh, dt);

    float hp01 = hud2_clampf((float)s->health / (float)MAX_HEALTH, 0.0f, 1.0f);
    hud2_vignette_update(&h->vignette, hp01, dt);

    hud2_xpbar_update(&h->xpbar, dt);
    hud2_cooldown_update(&h->cooldown, h->swing_recover, dt);
}

void hud2_set_xp(hud2 *h, int total_xp) {
    hud2_xpbar_set(&h->xpbar, total_xp);
}

// the gl side: set the screen-size uniforms, enable blending, draw the batch,
// restore. we keep depth testing off for the whole overlay.
static void gl_overlay_begin(hud2 *h) {
    glUseProgram(h->prog);
    gl_set_uniform_float(h->prog, "u_sw", (float)h->sw);
    gl_set_uniform_float(h->prog, "u_sh", (float)h->sh);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void gl_overlay_end(void) {
    glEnable(GL_DEPTH_TEST);
}

void hud2_render(hud2 *h, const inventory *inv, const survival *s) {
    (void)s;
    int sw = h->sw, sh = h->sh;

    gl_overlay_begin(h);
    hud2_batch_begin(&h->batch, sw, sh);

    // draw order matters: the screen-edge overlays go down first so widgets
    // sit on top of them, then the bottom cluster, then crosshair, then the
    // corner toasts last so nothing covers them.
    hud2_vignette_draw(&h->vignette, &h->batch, sw, sh);

    float base_y = hud2_bars_base_y(sw, sh);
    hud2_bars_draw(&h->bars, &h->batch, sw, sh, base_y);

    // xp strip sits just above the bars row.
    hud2_xpbar_draw(&h->xpbar, &h->batch, sw, base_y - 12.0f);

    hud2_hotbar_draw(&h->hotbar, &h->batch, inv);

    hud2_crosshair_draw(&h->crosshair, &h->batch, sw, sh);
    // the cooldown arc rides just below the crosshair center.
    hud2_cooldown_draw(&h->cooldown, &h->batch, sw * 0.5f, sh * 0.5f, 16.0f);
    hud2_toast_draw(&h->toasts, &h->batch);

    hud2_batch_end(&h->batch);
    gl_overlay_end();

    // text is a separate renderer with its own program, so it draws after the
    // batch flush. nothing batches across the two; thats fine at hud volumes.
    if (!h->text) return;

    // hotbar selection label, centered above the hotbar, fading.
    float lbl_a = 0.0f;
    const char *lbl = hud2_hotbar_label(&h->hotbar, inv, &lbl_a);
    if (lbl && lbl[0] && lbl_a > 0.01f) {
        // crude centering: the bitmap font is fixed-width, count chars.
        int n = 0; while (lbl[n]) n++;
        int approx_w = n * 8;     // ~8px per glyph in this font
        int lx = sw / 2 - approx_w / 2;
        int ly = (int)(h->hotbar.origin_y - 24.0f);
        text_draw(h->text, lbl, lx, ly, lbl_a, lbl_a, lbl_a, sw, sh);
    }

    // xp level number, centered over the xp strip. greener while dinging.
    float ding = 0.0f;
    int level = hud2_xpbar_level(&h->xpbar, &ding);
    if (level > 0) {
        char num[8];
        int n = 0, lv = level;
        if (lv > 999) lv = 999;
        // tiny itoa; the font has no printf and i didnt want stdio here.
        char tmp[8]; int ti = 0;
        do { tmp[ti++] = (char)('0' + lv % 10); lv /= 10; } while (lv > 0);
        while (ti > 0) num[n++] = tmp[--ti];
        num[n] = 0;

        int lx = sw / 2 - (n * 8) / 2;
        int ly = (int)(hud2_bars_base_y(sw, sh) - 12.0f - 12.0f);
        float g = 0.85f + 0.15f * (ding > 0.0f ? 1.0f : 0.0f);
        text_draw(h->text, num, lx, ly, 0.6f, g, 0.5f, sw, sh);
    }

    // toast labels, one per live slot.
    for (int i = 0; i < HUD2_TOAST_MAX; i++) {
        const char *txt; float tx, ty, ta;
        if (!hud2_toast_text_at(&h->toasts, i, &txt, &tx, &ty, &ta))
            continue;
        text_draw(h->text, txt, (int)tx, (int)ty, ta, ta, ta, sw, sh);
    }
}

// ---- events ----

void hud2_on_swing(hud2 *h) {
    hud2_crosshair_recoil(&h->crosshair, 7.0f);
    hud2_cooldown_trigger(&h->cooldown);
}

void hud2_on_hit_confirm(hud2 *h) {
    hud2_crosshair_hit(&h->crosshair);
    // a small extra kick so a confirmed break feels punchier than a whiff.
    hud2_crosshair_recoil(&h->crosshair, 3.0f);
}

void hud2_on_pickup(hud2 *h, block_id id, int amount) {
    if (id == BLOCK_AIR || amount <= 0) return;
    hud2_toast_pickup(&h->toasts, id, amount);
}

void hud2_on_damage(hud2 *h, int amount, vec3 player_pos, float yaw,
                    vec3 source) {
    if (amount <= 0) return;
    float dir = hud2_hurt_dir_from_world(player_pos, yaw, source);
    float str = hud2_hurt_strength(amount);
    hud2_vignette_hurt(&h->vignette, dir, str);

    // big hits also throw a toast so the reason is legible in the log corner.
    if (amount >= 4)
        hud2_notify(h, "ouch", HUD2_TOAST_BAD);
}

void hud2_notify(hud2 *h, const char *text, hud2_toast_kind kind) {
    hud2_toast_push(&h->toasts, text, kind, BLOCK_AIR, 2.5f);
}
