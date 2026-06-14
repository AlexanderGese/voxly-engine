#include "colorlight_debug.h"
#include "colorlight_access.h"
#include "colorlight_emitter.h"
#include "../block.h"
#include "../../config.h"
#include "../../util/log.h"

#include <string.h>

static const int DX[6] = { 1,-1, 0, 0, 0, 0};
static const int DY[6] = { 0, 0, 1,-1, 0, 0};
static const int DZ[6] = { 0, 0, 0, 0, 1,-1};

void colorlight_debug_chunk_stats(world *w, chunk *c, colorlight_stats *out) {
    memset(out, 0, sizeof *out);
    colorlight_grid *g = colorlight_grid_peek(c);
    if (!g) return;

    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;

    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                colorlight_packed p = colorlight_grid_get(g, lx, y, lz);
                uint8_t r = colorlight_packed_r(p);
                uint8_t gg = colorlight_packed_g(p);
                uint8_t b = colorlight_packed_b(p);
                int peak = colorlight_packed_peak(p);
                if (peak == 0) continue;

                out->lit_cells++;
                if (r  > out->max_r) out->max_r = r;
                if (gg > out->max_g) out->max_g = gg;
                if (b  > out->max_b) out->max_b = b;
                if (peak > out->peak_any) out->peak_any = peak;

                // light sitting inside an opaque block means a write got the
                // bounds wrong somewhere. shouldn't ever happen.
                block_id id = chunk_get_block(c, lx, y, lz);
                if (block_is_opaque(id)) out->leak_count++;

                // gradient check: a lit cell that's not a source must drop by
                // exactly 1 to at least one neighbor on its peak channel.
                if (!colorlight_emitter_is(id)) {
                    int chan = (r >= gg && r >= b) ? 0 : (gg >= b ? 1 : 2);
                    int lv = colorlight_packed_chan(p, chan);
                    int ok = 0;
                    for (int d = 0; d < 6; d++) {
                        int nx = wx0 + lx + DX[d];
                        int ny = y + DY[d];
                        int nz = wz0 + lz + DZ[d];
                        if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
                        int nl = colorlight_world_get_chan(w, nx, ny, nz, chan);
                        if (nl >= lv + 1) { ok = 1; break; }
                    }
                    if (!ok && lv > 0) out->gradient_breaks++;
                }
            }
        }
    }
    out->fill_ratio = (float)out->lit_cells / (float)CHUNK_VOLUME;
}

int colorlight_debug_verify(world *w, chunk *c) {
    colorlight_stats st;
    colorlight_debug_chunk_stats(w, c, &st);
    if (st.leak_count == 0 && st.gradient_breaks == 0) return 1;
    LOGW("colorlight verify FAIL chunk %d,%d: %d leaks, %d gradient breaks",
         c->cx, c->cz, st.leak_count, st.gradient_breaks);
    return 0;
}

void colorlight_debug_dump_slice(chunk *c, int y, int chan) {
    colorlight_grid *g = colorlight_grid_peek(c);
    if (!g) { LOGD("colorlight: chunk %d,%d has no grid", c->cx, c->cz); return; }
    if (y < 0 || y >= CHUNK_SIZE_Y) return;

    static const char hex[] = "0123456789abcdef";
    const char *cname = chan == 0 ? "r" : chan == 1 ? "g" : "b";
    LOGD("colorlight slice chunk %d,%d y=%d chan=%s", c->cx, c->cz, y, cname);

    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        char row[CHUNK_SIZE_X + 1];
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            uint8_t v = colorlight_grid_get_chan(g, lx, y, lz, chan);
            row[lx] = v ? hex[v & 0x0F] : '.';
        }
        row[CHUNK_SIZE_X] = '\0';
        LOGD("  %s", row);
    }
}
