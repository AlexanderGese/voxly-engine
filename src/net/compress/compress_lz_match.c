#include "compress_lz_match.h"

#define LZ_MASK (COMPRESS_LZ_WINDOW - 1)

// 3-byte hash. cheap multiplicative mix, shift to the table width. collisions
// are fine, the chain walk verifies the actual bytes.
static uint32_t lz_hash(const uint8_t *p) {
    uint32_t h = (uint32_t)p[0]
               | ((uint32_t)p[1] << 8)
               | ((uint32_t)p[2] << 16);
    h *= 2654435761u;            // knuth's multiplicative constant
    return h >> (32 - COMPRESS_LZ_HASH_BITS);
}

void compress_lz_finder_init(compress_lz_finder *f,
                             const uint8_t *src, size_t len) {
    f->src = src;
    f->len = len;
    for (uint32_t i = 0; i < COMPRESS_LZ_HASH_SIZE; i++) f->head[i] = -1;
    for (uint32_t i = 0; i < COMPRESS_LZ_WINDOW; i++)    f->prev[i] = -1;
}

void compress_lz_finder_insert(compress_lz_finder *f, size_t pos) {
    if (pos + COMPRESS_LZ_MIN_MATCH > f->len) return; // not enough to hash
    uint32_t h = lz_hash(f->src + pos);
    f->prev[pos & LZ_MASK] = f->head[h];
    f->head[h] = (int32_t)pos;
}

static size_t match_len(const uint8_t *a, const uint8_t *b, size_t max) {
    size_t n = 0;
    while (n < max && a[n] == b[n]) n++;
    return n;
}

compress_lz_match compress_lz_finder_best(compress_lz_finder *f, size_t pos) {
    compress_lz_match best = { 0, 0 };

    size_t remaining = f->len - pos;
    if (remaining < COMPRESS_LZ_MIN_MATCH) return best;

    size_t max_match = remaining;
    if (max_match > COMPRESS_LZ_MAX_MATCH) max_match = COMPRESS_LZ_MAX_MATCH;

    uint32_t h = lz_hash(f->src + pos);
    int32_t cand = f->head[h];

    int chain = COMPRESS_LZ_MAX_CHAIN;
    const uint8_t *cur = f->src + pos;

    while (cand >= 0 && chain-- > 0) {
        size_t cpos = (size_t)cand;
        // window guard: positions older than the window are no longer
        // addressable by a distance varint we care about.
        if (pos - cpos > COMPRESS_LZ_WINDOW) break;

        // quick reject: if we already have a best, check the byte that would
        // extend it before doing the full compare.
        if (best.length == 0 || cur[best.length] == f->src[cpos + best.length]) {
            size_t ml = match_len(cur, f->src + cpos, max_match);
            if (ml > best.length) {
                best.length = ml;
                best.distance = pos - cpos;
                if (ml >= max_match) break;   // cant do better than the cap
            }
        }
        cand = f->prev[cpos & LZ_MASK];
    }

    if (best.length < COMPRESS_LZ_MIN_MATCH) {
        best.length = 0;
        best.distance = 0;
    }
    return best;
}
