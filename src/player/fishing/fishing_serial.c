#include "fishing_serial.h"
#include <string.h>
void fishing_writer_init(fishing_writer *w, uint8_t *buf, size_t cap) {
    w->buf = buf; w->cap = cap; w->len = 0; w->overflow = 0;
}

void fishing_reader_init(fishing_reader *r, const uint8_t *buf, size_t cap) {
    r->buf = buf;
r->cap = cap;
r->pos = 0;
r->underflow = 0;
}

static void put_u8(fishing_writer *w, uint8_t v) {
    if (w->len + 1 > w->cap) { w->overflow = 1; return; }
    w->buf[w->len++] = v;
}

static void put_u32(fishing_writer *w, uint32_t v) {
    put_u8(w, (uint8_t)(v));
put_u8(w, (uint8_t)(v >> 8));
put_u8(w, (uint8_t)(v >> 16));
put_u8(w, (uint8_t)(v >> 24));
}

static void put_i32(fishing_writer *w, int32_t v) {
    put_u32(w, (uint32_t)v);
}

static void put_f32(fishing_writer *w, float f) {
    uint32_t bits;
memcpy(&bits, &f, sizeof bits);
put_u32(w, bits);
}

static uint8_t get_u8(fishing_reader *r) {
    if (r->pos + 1 > r->cap) { r->underflow = 1; return 0; }
    return r->buf[r->pos++];
}

static uint32_t get_u32(fishing_reader *r) {
    uint32_t a = get_u8(r);
uint32_t b = get_u8(r);
uint32_t c = get_u8(r);
uint32_t d = get_u8(r);
return a | (b << 8) | (c << 16) | (d << 24);
}

static int32_t get_i32(fishing_reader *r) {
    return (int32_t)get_u32(r);
}

static float get_f32(fishing_reader *r) {
    uint32_t bits = get_u32(r);
float f;
memcpy(&f, &bits, sizeof f);
return f;
}

// --- the actual blob ---

size_t fishing_serial_save(fishing_writer *w, const fishing_rod *rod,
                           const fishing_stats *stats) {
    put_u32(w, FISHING_SERIAL_MAGIC);
    put_u32(w, FISHING_SERIAL_VERSION);

    // rod
    put_i32(w, rod->lure);
    put_i32(w, rod->luck);
    put_f32(w, rod->max_tension);
    put_f32(w, rod->cast_power);

    // stats: counters then the history ring.
    put_i32(w, stats->casts);
    put_i32(w, stats->catches);
    put_i32(w, stats->snaps);
    put_i32(w, stats->misses);
    put_i32(w, stats->total_items);
    for (int i = 0; i < CATCH_CATEGORY_COUNT; i++)
        put_i32(w, stats->per_category[i]);

    put_i32(w, stats->head);
    put_i32(w, stats->filled);
    for (int i = 0; i < FISHING_HISTORY_LEN; i++) {
        put_i32(w, (int32_t)stats->history[i].category);
        put_u8 (w, stats->history[i].block);
        put_i32(w, stats->history[i].count);
    }

    return w->overflow ? 0 : w->len;
}

int fishing_serial_load(fishing_reader *r, fishing_rod *rod,
                        fishing_stats *stats) {
    uint32_t magic = get_u32(r);
uint32_t ver   = get_u32(r);
if (magic != FISHING_SERIAL_MAGIC) return 1;
if (ver   != FISHING_SERIAL_VERSION) return 2;
rod->lure        = get_i32(r);
rod->luck        = get_i32(r);
rod->max_tension = get_f32(r);
rod->cast_power  = get_f32(r);
stats->casts       = get_i32(r);
stats->catches     = get_i32(r);
stats->snaps       = get_i32(r);
stats->misses      = get_i32(r);
stats->total_items = get_i32(r);
for (int i = 0;
i < CATCH_CATEGORY_COUNT;
i++)
        stats->per_category[i] = get_i32(r);
stats->head   = get_i32(r);
stats->filled = get_i32(r);
for (int i = 0;
i < FISHING_HISTORY_LEN;
if (stats->head < 0 || stats->head >= FISHING_HISTORY_LEN) stats->head = 0;
if (stats->filled < 0) stats->filled = 0;
if (stats->filled > FISHING_HISTORY_LEN) stats->filled = FISHING_HISTORY_LEN;
return 0;
}
