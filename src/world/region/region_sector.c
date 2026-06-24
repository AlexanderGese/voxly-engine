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
region_alloc_mark(a, 0, REGION_HEADER_SECTORS, 1);
ensure_cap(a, start + count - 1);
for (uint32_t s = start;
s < start + count;
uint32_t run_start = REGION_HEADER_SECTORS;
uint32_t run = 0;
for (uint32_t s = REGION_HEADER_SECTORS;
s < a->high;
region_alloc_mark(a, at, count, 1);
return at;
}
