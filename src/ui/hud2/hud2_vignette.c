#include "hud2_vignette.h"
#include "hud2_anim.h"
#include <math.h>

void hud2_vignette_init(hud2_vignette *v) {
    v->hurt      = 0.0f;
    v->hurt_dir  = 0.0f;
    v->low_pulse = 0.0f;
    v->low_amt   = 0.0f;
    v->inited    = 1;
}

void hud2_vignette_hurt(hud2_vignette *v, float dir, float strength) {
    if (!v->inited) hud2_vignette_init(v);
    strength = hud2_clampf(strength, 0.0f, 1.0f);
    // take the stronger of any in-flight flash so rapid hits dont cancel.
    if (strength > v->hurt) {
        v->hurt     = strength;
        v->hurt_dir = dir;
    }
}

void hud2_vignette_update(hud2_vignette *v, float health01, float dt) {
    if (!v->inited) hud2_vignette_init(v);

    v->hurt = hud2_approach(v->hurt, 0.0f, 4.5f, dt);
    if (v->hurt < 0.003f) v->hurt = 0.0f;

    // the persistent vignette only appears below ~35% hp, ramping to full at 0.
    float want = 0.0f;
    if (health01 < 0.35f)
        want = hud2_smoothstep((0.35f - health01) / 0.35f);
    v->low_amt   = hud2_approach(v->low_amt, want, 3.0f, dt);
    v->low_pulse += dt;
}

// draw a falloff band stack hugging one edge. side: 0=top 1=bottom 2=left
// 3=right. peak is the alpha at the outer edge; it fades to 0 over `depth` px.
static void edge_bands(hud2_batch *b, int sw, int sh, int side,
                       float depth, hud2_color col, float peak) {
    if (peak <= 0.001f) return;
    float step = depth / HUD2_VIG_BANDS;
    for (int i = 0; i < HUD2_VIG_BANDS; i++) {
        // alpha falls off quadratically from the edge inward.
        float t = 1.0f - (float)i / (float)HUD2_VIG_BANDS;
        float a = peak * t * t;
        hud2_color c = hud2_color_fade(col, a);
        float off = i * step;
        switch (side) {
        case 0: hud2_batch_quad(b, 0, off, sw, step, c); break;            // top
        case 1: hud2_batch_quad(b, 0, sh - off - step, sw, step, c); break;// bottom
        case 2: hud2_batch_quad(b, off, 0, step, sh, c); break;           // left
        case 3: hud2_batch_quad(b, sw - off - step, 0, step, sh, c); break;// right
        }
    }
}

void hud2_vignette_draw(hud2_vignette *v, hud2_batch *b, int sw, int sh) {
    float depth = sh * 0.22f;

    // persistent low-hp vignette: dark red all four edges, throbbing slowly.
    if (v->low_amt > 0.001f) {
        float throb = 0.75f + 0.25f * sinf(v->low_pulse * 3.0f);
        float peak  = 0.55f * v->low_amt * throb;
        hud2_color red = hud2_rgb(0.55f, 0.02f, 0.02f);
        for (int s = 0; s < 4; s++)
            edge_bands(b, sw, sh, s, depth, red, peak);
    }

    // directional hurt flash: brighter red, weighted toward the hit side. we
    // map the hit angle onto per-side weights via a cosine lobe so a hit from
    // the right lights the right edge most, with some bleed to neighbors.
    if (v->hurt > 0.001f) {
        hud2_color hr = hud2_rgb(0.85f, 0.05f, 0.05f);
        // side facing angles: right=0, top=-pi/2, left=pi, bottom=pi/2.
        float side_ang[4] = { -1.5708f, 1.5708f, 3.14159f, 0.0f };
        for (int s = 0; s < 4; s++) {
            float d = v->hurt_dir - side_ang[s];
            // wrap to -pi..pi
            while (d >  3.14159f) d -= 6.28318f;
            while (d < -3.14159f) d += 6.28318f;
            float lobe = 0.5f + 0.5f * cosf(d);   // 1 facing, 0 opposite
            float peak = 0.7f * v->hurt * (0.3f + 0.7f * lobe * lobe);
            edge_bands(b, sw, sh, s, depth * 1.1f, hr, peak);
        }
    }
}
