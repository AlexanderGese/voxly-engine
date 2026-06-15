#include "colorlight_serialize.h"
#include "../../config.h"

#include <string.h>

// little-endian put/get helpers. the rest of the save code assumes LE host and
// i'm not going to be the one file that's portable about it.
static void put_u16(uint8_t **p, uint16_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
    (*p)[1] = (uint8_t)(v >> 8);
    *p += 2;
}

static void put_u32(uint8_t **p, uint32_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
    (*p)[1] = (uint8_t)((v >> 8) & 0xFF);
    (*p)[2] = (uint8_t)((v >> 16) & 0xFF);
    (*p)[3] = (uint8_t)((v >> 24) & 0xFF);
    *p += 4;
}

static uint16_t get_u16(const uint8_t **p) {
    uint16_t v = (uint16_t)((*p)[0] | ((*p)[1] << 8));
    *p += 2;
    return v;
}

static uint32_t get_u32(const uint8_t **p) {
    uint32_t v = (uint32_t)((*p)[0]) | ((uint32_t)(*p)[1] << 8)
               | ((uint32_t)(*p)[2] << 16) | ((uint32_t)(*p)[3] << 24);
    *p += 4;
    return v;
}

size_t colorlight_serialize_max_bytes(void) {
    // header (4+2+2) + worst case CHUNK_VOLUME runs of (2+2).
    return 8 + (size_t)CHUNK_VOLUME * 4;
}

size_t colorlight_serialize_encode(const colorlight_grid *g, uint8_t *dst, size_t cap) {
    if (!g || !dst) return 0;
    if (cap < colorlight_serialize_max_bytes()) return 0;

    uint8_t *p = dst;
    put_u32(&p, COLORLIGHT_SAVE_MAGIC);
    put_u16(&p, COLORLIGHT_SAVE_VERSION);

    // remember where the run count goes, backfill it once we know.
    uint8_t *count_at = p;
    put_u16(&p, 0);

    uint16_t runs = 0;
    int i = 0;
    while (i < CHUNK_VOLUME) {
        colorlight_packed v = g->cells[i];
        int j = i + 1;
        // runs cap at 0xFFFF; a full dark chunk (CHUNK_VOLUME) exceeds that, so
        // split long runs. CHUNK_VOLUME is 32768 here, but keep the guard honest.
        while (j < CHUNK_VOLUME && g->cells[j] == v && (j - i) < 0xFFFF) j++;
        put_u16(&p, v);
        put_u16(&p, (uint16_t)(j - i));
        runs++;
        i = j;
    }

    // backfill run count.
    count_at[0] = (uint8_t)(runs & 0xFF);
    count_at[1] = (uint8_t)(runs >> 8);

    return (size_t)(p - dst);
}

size_t colorlight_serialize_decode(colorlight_grid *g, const uint8_t *src, size_t len) {
    if (!g || !src || len < 8) return 0;

    const uint8_t *p = src;
    if (get_u32(&p) != COLORLIGHT_SAVE_MAGIC) return 0;
    uint16_t ver = get_u16(&p);
    if (ver != COLORLIGHT_SAVE_VERSION) return 0;
    uint16_t runs = get_u16(&p);

    int idx = 0;
    for (uint16_t r = 0; r < runs; r++) {
        // bounds check before each run read.
        if ((size_t)(p - src) + 4 > len) return 0;
        colorlight_packed v = (colorlight_packed)get_u16(&p);
        uint16_t cnt = get_u16(&p);
        if (idx + cnt > CHUNK_VOLUME) return 0; // corrupt, refuse
        for (int k = 0; k < cnt; k++) g->cells[idx++] = v;
    }
    if (idx != CHUNK_VOLUME) return 0; // didn't fill the chunk, reject
    g->dirty = 1;
    return (size_t)(p - src);
}
