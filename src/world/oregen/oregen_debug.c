#include "oregen_debug.h"
#include "oregen_table.h"
#include "../../config.h"

int oregen_debug_height_hist(const oregen_buf *buf, int *out, int max_y) {
    if (!buf || !out || max_y <= 0) return 0;
    for (int y = 0; y < max_y; y++) out[y] = 0;

    int total = 0;
    for (int i = 0; i < buf->count; i++) {
        int y = buf->items[i].y;
        if (y < 0 || y >= max_y) continue;
        out[y]++;
        total++;
    }
    return total;
}

int oregen_debug_block_counts(const oregen_buf *buf, int *out, int out_len) {
    if (!buf || !out || out_len <= 0) return 0;
    for (int i = 0; i < out_len; i++) out[i] = 0;

    for (int i = 0; i < buf->count; i++) {
        block_id id = buf->items[i].id;
        if (id < out_len) out[id]++;
    }

    int distinct = 0;
    for (int i = 0; i < out_len; i++) if (out[i] > 0) distinct++;
    return distinct;
}

float oregen_debug_band_fraction(const oregen_buf *buf, int y_lo, int y_hi) {
    if (!buf || buf->count == 0) return 0.0f;
    if (y_hi < y_lo) { int t = y_lo; y_lo = y_hi; y_hi = t; }

    int in = 0;
    for (int i = 0; i < buf->count; i++) {
        int y = buf->items[i].y;
        if (y >= y_lo && y <= y_hi) in++;
    }
    return (float)in / (float)buf->count;
}

int oregen_debug_validate_table(void) {
    int n = oregen_table_count();
    for (int i = 0; i < n; i++) {
        const oregen_ore *o = oregen_table_at(i);
        if (o->y_min > o->y_max)        return -(i + 1);
        if (o->y_min < 0)               return -(i + 1);
        if (o->y_max >= CHUNK_SIZE_Y)   return -(i + 1);
        if (o->size_min < 1)            return -(i + 1);
        if (o->size_max < o->size_min)  return -(i + 1);
        if (o->tries_per_chunk < 0.0f)  return -(i + 1);
        if (o->squish <= 0.0f)          return -(i + 1);
        if (o->curve < 0 || o->curve >= OREGEN_CURVE_COUNT) return -(i + 1);
        if (o->shape < 0 || o->shape >= OREGEN_SHAPE_COUNT) return -(i + 1);
    }
    return 0;
}
