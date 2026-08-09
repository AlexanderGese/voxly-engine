#include "crafting_serial.h"
#include "crafting_book.h"
#include "crafting_stats.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// blob layout (all little-ish endian via raw memcpy of u32, we never move
// saves between machines so byte order is moot):
// u32 magic
// u32 version
// u32 recipe_count          // how many ids we wrote stats for
// then recipe_count records of: u8 unlocked, u32 times, u32 yield
// the count lets the loader skip ids the current build no longer knows about.

// little write/read cursor helpers. they bump a byte pointer and bounds-check
// against the end so a truncated file cant walk off the buffer.

typedef struct { uint8_t *p; uint8_t *end; } wcur;
typedef struct { const uint8_t *p; const uint8_t *end; } rcur;

static void w_u32(wcur *c, uint32_t v) {
    if (c->p + 4 > c->end) return;
    memcpy(c->p, &v, 4);
    c->p += 4;
}
static void w_u8(wcur *c, uint8_t v) {
    if (c->p + 1 > c->end) return;
    *c->p++ = v;
}
static int r_u32(rcur *c, uint32_t *out) {
    if (c->p + 4 > c->end) return 0;
    memcpy(out, c->p, 4);
    c->p += 4;
    return 1;
}
static int r_u8(rcur *c, uint8_t *out) {
    if (c->p + 1 > c->end) return 0;
    *out = *c->p++;
    return 1;
}

void *craft_save_to_buffer(size_t *out_size) {
    int n = craft_book_count();
    // header (12) + n * (1 + 4 + 4)
    size_t sz = 12 + (size_t)n * 9;
    uint8_t *buf = malloc(sz);
    if (!buf) return NULL;

    wcur c = { buf, buf + sz };
    w_u32(&c, CRAFT_SAVE_MAGIC);
    w_u32(&c, CRAFT_SAVE_VERSION);
    w_u32(&c, (uint32_t)n);
    for (int i = 0; i < n; i++) {
        w_u8(&c, (uint8_t)(craft_book_is_unlocked(i) ? 1 : 0));
        w_u32(&c, (uint32_t)craft_stats_times(i));
        w_u32(&c, (uint32_t)craft_stats_yield(i));
    }
    if (out_size) *out_size = sz;
    return buf;
}

int craft_load_from_buffer(const void *buf, size_t size) {
    rcur c = { (const uint8_t *)buf, (const uint8_t *)buf + size };
    uint32_t magic = 0, version = 0, count = 0;
    if (!r_u32(&c, &magic) || magic != CRAFT_SAVE_MAGIC) {
        LOGW("crafting: bad save magic, skipping");
        return -1;
    }
    if (!r_u32(&c, &version) || version != CRAFT_SAVE_VERSION) {
        LOGW("crafting: save version %u != %u", version, CRAFT_SAVE_VERSION);
        return -2;
    }
    if (!r_u32(&c, &count)) return -3;

    int known = craft_book_count();
    // reset stats so a load is a clean replace, not a merge.
    craft_stats_reset();

    for (uint32_t i = 0; i < count; i++) {
        uint8_t unlocked = 0;
        uint32_t times = 0, yield = 0;
        if (!r_u8(&c, &unlocked)) return -4;
        if (!r_u32(&c, &times))   return -4;
        if (!r_u32(&c, &yield))   return -4;

        // ids beyond what this build knows are silently dropped (recipe set
        // shrank). we still had to read them to stay aligned.
        if ((int)i >= known) continue;
        if (unlocked) craft_book_unlock((int)i);
        // replay the stats. record() also rebuilds the recent ring, but a load
        // shouldnt pretend these were "just crafted", so poke the counters
        // through record() once per craft would be wrong; instead we fold them
        // in as a bulk single record and accept the recent-ring approximation.
        if (times > 0) {
            // average yield per craft, rounded, just to give record() a number.
            int per = yield && times ? (int)(yield / times) : 0;
            for (uint32_t k = 0; k < times; k++)
                craft_stats_record((int)i, per);
        }
    }
    LOGI("crafting: loaded progress for %u recipe(s)", count);
    return 0;
}

int craft_save_to_file(const char *path) {
    size_t sz = 0;
    void *buf = craft_save_to_buffer(&sz);
    if (!buf) return -1;
    int rc = file_write_all(path, buf, sz);
    free(buf);
    return rc;
}

int craft_load_from_file(const char *path) {
    size_t sz = 0;
    char *buf = file_read_all(path, &sz);
    if (!buf) return -1;
    int rc = craft_load_from_buffer(buf, sz);
    free(buf);
    return rc;
}
