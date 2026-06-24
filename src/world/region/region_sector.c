#include "region_sector.h"
#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>

#define BIT_WORD(s)  ((s) >> 3)
#define BIT_MASK(s)  (1u << ((s) & 7))

static void ensure_cap(region_alloc_t *a, uint32_t sector) {
    if (sector < a->cap) return;
    uint32_t newcap = a->cap ? a->cap : 256;
    while (newcap <= sector) newcap *= 2;
    uint8_t *nb = realloc(a->bits, BIT_WORD(newcap) + 1);
    if (!nb) { LOGE("region: sector bitmap oom"); return; }
    // zero the freshly grown tail
    size_t old_bytes = a->cap ? BIT_WORD(a->cap) + 1 : 0;
    size_t new_bytes = BIT_WORD(newcap) + 1;
    memset(nb + old_bytes, 0, new_bytes - old_bytes);
    a->bits = nb;
    a->cap = newcap;
}

void region_alloc_init(region_alloc_t *a) {
    a->bits = NULL;
    a->cap = 0;
    a->high = REGION_HEADER_SECTORS;
    // header sectors are reserved forever.
    region_alloc_mark(a, 0, REGION_HEADER_SECTORS, 1);
}

void region_alloc_free(region_alloc_t *a) {
    free(a->bits);
    a->bits = NULL;
    a->cap = 0;
    a->high = 0;
}

void region_alloc_mark(region_alloc_t *a, uint32_t start, uint32_t count, int used) {
    if (!count) return;
    ensure_cap(a, start + count - 1);
    for (uint32_t s = start; s < start + count; s++) {
        if (used) a->bits[BIT_WORD(s)] |=  BIT_MASK(s);
        else      a->bits[BIT_WORD(s)] &= ~BIT_MASK(s);
    }
    if (used && start + count > a->high) a->high = start + count;
}

int region_alloc_is_used(const region_alloc_t *a, uint32_t sector) {
    if (sector >= a->cap) return 0;
    return (a->bits[BIT_WORD(sector)] & BIT_MASK(sector)) != 0;
}

uint32_t region_alloc_reserve(region_alloc_t *a, uint32_t count) {
    if (count == 0) count = 1;

    // first-fit scan over the currently allocated span. start past the header,
    // we never hand those out.
    uint32_t run_start = REGION_HEADER_SECTORS;
    uint32_t run = 0;
    for (uint32_t s = REGION_HEADER_SECTORS; s < a->high; s++) {
        if (region_alloc_is_used(a, s)) {
            run = 0;
            run_start = s + 1;
        } else {
            run++;
            if (run == count) {
                region_alloc_mark(a, run_start, count, 1);
                return run_start;
            }
        }
    }

    // no gap big enough, append at the end. this grows the file.
    uint32_t at = a->high;
    region_alloc_mark(a, at, count, 1);
    return at;
}

void region_alloc_release(region_alloc_t *a, uint32_t start, uint32_t count) {
    if (start < REGION_HEADER_SECTORS) {
        // someone tried to free the header. ignore, thats a bug upstream.
        LOGW("region: refused to release header sectors at %u", start);
        return;
    }
    region_alloc_mark(a, start, count, 0);

    // pull `high` back down if we just freed the tail. keeps files from
    // bloating after a bunch of chunks get deleted.
    while (a->high > REGION_HEADER_SECTORS &&
           !region_alloc_is_used(a, a->high - 1)) {
        a->high--;
    }
}

uint32_t region_alloc_high(const region_alloc_t *a) {
    return a->high;
}
