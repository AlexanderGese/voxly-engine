#include "lightprop_debug.h"
#include "lightprop_access.h"
#include "lightprop_step.h"
#include "../../util/log.h"

#include <string.h>

void lp_collect_stats(world *w, chunk *c, lp_stats *out) {
    (void)w;
    memset(out, 0, sizeof *out);
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++)
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                uint8_t b = chunk_get_blocklight(c, lx, y, lz);
                uint8_t s = chunk_get_sunlight(c, lx, y, lz);
                if (b) { out->lit_block++; if (b > out->max_block) out->max_block = b; }
                if (s) { out->lit_sky++;   if (s > out->max_sky)   out->max_sky   = s; }
            }
}

int lp_verify_chunk(world *w, chunk *c, lp_channel ch) {
    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;
    int bad = 0;

    for (int y = 0; y < CHUNK_SIZE_Y; y++)
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++)
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                block_id id = chunk_get_block(c, lx, y, lz);
                if (!lp_transmits(id)) continue;   // opaque cells exempt

                int wx = wx0 + lx, wz = wz0 + lz;
                uint8_t cur = lp_get_light(w, ch, wx, y, wz);

                // find the strongest level a neighbour would feed us.
                uint8_t best = 0;
                for (int d = 0; d < 6; d++) {
                    int nx = wx + LP_DX[d], ny = y + LP_DY[d], nz = wz + LP_DZ[d];
                    if (!lp_y_in_range(ny)) continue;
                    if (!lp_cell_loaded(w, nx, nz)) continue;
                    uint8_t nl = lp_get_light(w, ch, nx, ny, nz);
                    // reverse the step: what would the neighbour push into us?
                    uint8_t given = lp_step_propagate(ch, d ^ 1, nl, id);
                    if (given > best) best = given;
                }

                // a cell with no source should never be brighter than its best
                // feeder, and should never be dimmer either (flood reaches all).
                uint8_t emit = ch == LP_BLOCK ? 0 : 0; // sources handled elsewhere
                (void)emit;
                if (cur > best && cur > 0 && best > 0 && cur != MAX_LIGHT) {
                    if (bad < 8)
                        LOGW("lightprop verify: ghost at (%d,%d,%d) cur=%u best=%u",
                             wx, y, wz, cur, best);
                    bad++;
                }
            }

    if (bad) LOGW("lightprop verify: %d suspect cells in chunk (%d,%d) ch=%d",
                  bad, c->cx, c->cz, (int)ch);
    return bad;
}

void lp_dump_slice(world *w, chunk *c, int lz, lp_channel ch) {
    (void)w;
    if (lz < 0 || lz >= CHUNK_SIZE_Z) return;
    static const char ramp[] = "0123456789abcdef";
    // top 24 rows is plenty to eyeball a torch; full column spams the log.
    int ytop = CHUNK_SIZE_Y - 1;
    int ybot = ytop - 24; if (ybot < 0) ybot = 0;
    LOGD("lightprop slice chunk(%d,%d) z=%d ch=%d:", c->cx, c->cz, lz, (int)ch);
    for (int y = ytop; y >= ybot; y--) {
        char row[CHUNK_SIZE_X + 1];
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            uint8_t v = ch == LP_SKY ? chunk_get_sunlight(c, lx, y, lz)
                                     : chunk_get_blocklight(c, lx, y, lz);
            block_id id = chunk_get_block(c, lx, y, lz);
            row[lx] = block_is_opaque(id) ? '#' : ramp[v & 0x0F];
        }
        row[CHUNK_SIZE_X] = 0;
        LOGD("  y%3d |%s|", y, row);
    }
}
