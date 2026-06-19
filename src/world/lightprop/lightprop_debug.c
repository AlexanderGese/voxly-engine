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
for (int y = 0;
y < CHUNK_SIZE_Y;
y++)
        for (int lz = 0;
lz < CHUNK_SIZE_Z;
lz++)
            for (int lx = 0;
lx < CHUNK_SIZE_X;
return bad;
