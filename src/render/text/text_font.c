#include "text_font.h"
#include "text_glyphdata.h"
#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>

// we render the 5x7 source at a fixed scale so the hud font isnt microscopic on
// a 1280x720 window. bump SCALE if you want chunkier text.
#define FONT_SCALE     2
#define GLYPH_PAD_R    1   // empty px after the bitmap, baked into advance

// scratch buffer for one glyph's expanded coverage bitmap.
// max footprint: COLS*SCALE wide, ROWS*SCALE tall.
#define MAX_GW (TEXT_GLYPH_COLS * FONT_SCALE)
#define MAX_GH (TEXT_GLYPH_ROWS * FONT_SCALE)

// find the rightmost used column across all 7 rows. lets us trim so glyphs are
// proportional. returns -1 for an all-blank glyph (space).
static int glyph_used_width(const uint8_t rows[TEXT_GLYPH_ROWS]) {
    int maxcol = -1;
    for (int r = 0; r < TEXT_GLYPH_ROWS; r++) {
        for (int c = 0; c < TEXT_GLYPH_COLS; c++) {
            int bit = (rows[r] >> (TEXT_GLYPH_COLS - 1 - c)) & 1;
            if (bit && c > maxcol) maxcol = c;
        }
    }
    return maxcol;
}

// expand a source glyph into a scaled a8 coverage buffer. cols is the trimmed
// source column count. writes scaled w*h, returns nothing (caller knows size).
static void rasterize_glyph(const uint8_t rows[TEXT_GLYPH_ROWS], int cols,
                            uint8_t *dst, int dst_w) {
    for (int r = 0; r < TEXT_GLYPH_ROWS; r++) {
        for (int c = 0; c < cols; c++) {
            int bit = (rows[r] >> (TEXT_GLYPH_COLS - 1 - c)) & 1;
            uint8_t v = bit ? 255 : 0;
            // nearest-expand the texel into a SCALE*SCALE block
            for (int sy = 0; sy < FONT_SCALE; sy++) {
                for (int sx = 0; sx < FONT_SCALE; sx++) {
                    int px = c * FONT_SCALE + sx;
                    int py = r * FONT_SCALE + sy;
                    dst[py * dst_w + px] = v;
                }
            }
        }
    }
}

void text_font_add_kern(text_font *f, uint32_t left, uint32_t right, int amount) {
    if (f->kern_count >= TEXT_MAX_KERN_PAIRS) return;
    if (amount < -128) amount = -128;
    if (amount >  127) amount =  127;
    text_kern *k = &f->kern[f->kern_count++];
    k->left   = (uint8_t)left;
    k->right  = (uint8_t)right;
    k->amount = (int8_t)amount;
}

// a handful of pairs that look bad without tightening. not exhaustive, just the
// ones that bugged me in actual hud strings.
static void install_default_kerning(text_font *f) {
    text_font_add_kern(f, 'A', 'V', -FONT_SCALE);
    text_font_add_kern(f, 'V', 'A', -FONT_SCALE);
    text_font_add_kern(f, 'A', 'W', -FONT_SCALE);
    text_font_add_kern(f, 'T', 'o', -FONT_SCALE);
    text_font_add_kern(f, 'T', 'a', -FONT_SCALE);
    text_font_add_kern(f, 'T', '.', -FONT_SCALE);
    text_font_add_kern(f, 'T', ',', -FONT_SCALE);
    text_font_add_kern(f, 'Y', 'o', -FONT_SCALE);
    text_font_add_kern(f, 'F', '.', -FONT_SCALE);
    text_font_add_kern(f, 'P', '.', -FONT_SCALE);
    text_font_add_kern(f, 'r', '.', -FONT_SCALE);
    text_font_add_kern(f, 'f', 'f', -FONT_SCALE);
    text_font_add_kern(f, 'L', 'T', -FONT_SCALE);
    text_font_add_kern(f, '.', '.', -FONT_SCALE);
}

int text_font_init_builtin(text_font *f) {
    memset(f, 0, sizeof *f);

    // atlas size: enough to hold ~95 glyphs of up to 10x14 with padding. 256^2
    // is overkill but powers of two keep gpus happy and we have the ram.
    if (!text_atlas_init(&f->atlas, 256, 128)) {
        LOGE("text_font: atlas init failed");
        return 0;
    }

    uint8_t scratch[MAX_GW * MAX_GH];

    for (int i = 0; i < TEXT_GLYPH_COUNT; i++) {
        const uint8_t *rows = TEXT_GLYPH_BITS[i];
        text_glyph *g = &f->glyphs[i];
        int used = glyph_used_width(rows);

        if (used < 0) {
            // blank glyph (space and friends). no bitmap, just advance.
            g->rect = (text_atlas_rect){0, 0, 0, 0};
            g->w = g->h = 0;
            g->bearing_x = 0;
            g->bearing_y = 0;
            g->advance = 3 * FONT_SCALE; // ~half an em-ish
            continue;
        }

        int cols = used + 1;
        int gw = cols * FONT_SCALE;
        int gh = TEXT_GLYPH_ROWS * FONT_SCALE;

        memset(scratch, 0, sizeof scratch);
        rasterize_glyph(rows, cols, scratch, gw);

        text_atlas_rect rect;
        if (!text_atlas_pack(&f->atlas, gw, gh, &rect)) {
            LOGE("text_font: atlas full at glyph %d ('%c')", i,
                 (char)(TEXT_FIRST_CP + i));
            text_atlas_destroy(&f->atlas);
            return 0;
        }
        text_atlas_blit_a8(&f->atlas, &rect, scratch, gw);

        g->rect      = rect;
        g->w         = gw;
        g->h         = gh;
        g->bearing_x = 0;
        g->bearing_y = gh;                 // whole glyph sits above baseline
        g->advance   = gw + GLYPH_PAD_R * FONT_SCALE;
    }

    // bake a 2x2 solid white block for panel backgrounds. uv'ing the center of
    // this gives a flat opaque sample regardless of filtering.
    {
        uint8_t solid[4 * 4];
        memset(solid, 255, sizeof solid); // 2x2 rgba8, all white opaque
        if (text_atlas_pack(&f->atlas, 2, 2, &f->white)) {
            text_atlas_blit_rgba(&f->atlas, &f->white, solid, 2 * 4);
        } else {
            f->white = (text_atlas_rect){0, 0, 0, 0};
        }
    }

    // line metrics. ascent = glyph height, small descent for the j/g/p tails,
    // which our 5x7 cells already include, so descent is modest.
    f->ascent        = TEXT_GLYPH_ROWS * FONT_SCALE;
    f->descent       = 2 * FONT_SCALE;
    f->line_gap      = 1 * FONT_SCALE;
    f->line_height   = f->ascent + f->descent + f->line_gap;
    f->space_advance = f->glyphs[' ' - TEXT_FIRST_CP].advance;
    f->tab_cols      = 4;

    install_default_kerning(f);

    text_atlas_upload(&f->atlas);
    f->baked = 1;
    return 1;
}

void text_font_destroy(text_font *f) {
    text_atlas_destroy(&f->atlas);
    memset(f, 0, sizeof *f);
}

const text_glyph *text_font_glyph(const text_font *f, uint32_t cp) {
    if (cp < TEXT_FIRST_CP || cp > TEXT_LAST_CP) {
        // tofu: reuse '?' so missing glyphs are at least visible
        return &f->glyphs['?' - TEXT_FIRST_CP];
    }
    return &f->glyphs[cp - TEXT_FIRST_CP];
}

int text_font_kern(const text_font *f, uint32_t left, uint32_t right) {
    if (left > 255 || right > 255) return 0;
    for (int i = 0; i < f->kern_count; i++) {
        if (f->kern[i].left == left && f->kern[i].right == right)
            return f->kern[i].amount;
    }
    return 0;
}
