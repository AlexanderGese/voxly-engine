#include "region_compact.h"
#include "region_coord.h"
#include "region_io.h"
#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>

float region_compact_fragmentation(const region_file_t *rf) {
    uint32_t live = 0;
    for (int slot = 0; slot < REGION_CHUNKS_PER_FILE; slot++) {
        if (!region_header_present(&rf->header, slot)) continue;
        live += region_header_loc(&rf->header, slot)->count;
    }
    uint32_t span = region_alloc_high(&rf->alloc);
    if (span <= REGION_HEADER_SECTORS) return 0.0f;
    uint32_t usable = span - REGION_HEADER_SECTORS;
    if (usable == 0) return 0.0f;
    uint32_t holes = usable - live;
    return (float)holes / (float)usable;
}

// a present slot and its source location, sorted by current offset so we move
// payloads front-to-back and never clobber one we havent copied yet.
typedef struct { int slot; uint32_t offset; uint8_t count; uint8_t enc; } live_t;

static int cmp_live(const void *a, const void *b) {
    uint32_t oa = ((const live_t*)a)->offset;
    uint32_t ob = ((const live_t*)b)->offset;
    return (oa > ob) - (oa < ob);
}

int region_compact_file(region_file_t *rf, region_compact_stats_t *stats) {
    region_compact_stats_t s;
    memset(&s, 0, sizeof s);
    s.sectors_before = region_alloc_high(&rf->alloc);

    // collect live payloads
    live_t *live = malloc(sizeof(live_t) * REGION_CHUNKS_PER_FILE);
    if (!live) return -1;
    int n = 0;
    for (int slot = 0; slot < REGION_CHUNKS_PER_FILE; slot++) {
        if (!region_header_present(&rf->header, slot)) continue;
        const region_loc_t *l = region_header_loc(&rf->header, slot);
        live[n].slot = slot;
        live[n].offset = l->offset;
        live[n].count = l->count;
        live[n].enc = l->enc;
        n++;
    }
    qsort(live, n, sizeof(live_t), cmp_live);

    // rebuild allocator from scratch: header only, then bump-pack.
    region_alloc_free(&rf->alloc);
    region_alloc_init(&rf->alloc);

    uint32_t write_at = REGION_HEADER_SECTORS;
    uint8_t  sbuf[REGION_SECTOR_BYTES];

    for (int i = 0; i < n; i++) {
        live_t *lv = &live[i];

        if (lv->offset != write_at) {
            // relocate: copy sector by sector from src to dst.
            for (uint32_t k = 0; k < lv->count; k++) {
                if (region_io_read_sectors(rf->f, lv->offset + k, 1, sbuf) != 0) {
                    free(live); return -1;
                }
                if (region_io_write_payload(rf->f, write_at + k, sbuf,
                                            REGION_SECTOR_BYTES) != 0) {
                    free(live); return -1;
                }
            }
            s.chunks_moved++;
        }

        region_alloc_mark(&rf->alloc, write_at, lv->count, 1);
        region_header_set(&rf->header, lv->slot, write_at, lv->count, lv->enc);
        write_at += lv->count;
    }

    region_header_write(&rf->header, rf->f);
    region_io_truncate(rf->f, write_at);

    s.sectors_after = write_at;
    s.sectors_freed = s.sectors_before > s.sectors_after
                    ? s.sectors_before - s.sectors_after : 0;
    free(live);

    if (stats) *stats = s;
    LOGI("region r.%d.%d: compacted, moved %u, freed %u sectors",
         rf->rx, rf->rz, s.chunks_moved, s.sectors_freed);
    return 0;
}

int region_compact_dir(const char *dir, float threshold) {
    DIR *d = opendir(dir);
    if (!d) { LOGW("region: cant scan dir %s", dir); return -1; }

    int rewritten = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        int rx, rz;
        if (!region_coord_parse(ent->d_name, &rx, &rz)) continue;

        region_file_t rf;
        if (region_file_open(&rf, dir, rx, rz, 0) != 0) continue;

        float frag = region_compact_fragmentation(&rf);
        if (frag >= threshold) {
            if (region_compact_file(&rf, NULL) == 0) rewritten++;
        }
        region_file_close(&rf);
    }
    closedir(d);
    return rewritten;
}

void region_compact_usage(const region_file_t *rf, region_usage_t *out) {
    int      live_chunks = 0;
    uint32_t live_sectors = 0;
    for (int slot = 0; slot < REGION_CHUNKS_PER_FILE; slot++) {
        if (!region_header_present(&rf->header, slot)) continue;
        live_chunks++;
        live_sectors += region_header_loc(&rf->header, slot)->count;
    }

    uint32_t span   = region_alloc_high(&rf->alloc);
    uint32_t usable = span > REGION_HEADER_SECTORS ? span - REGION_HEADER_SECTORS : 0;
    uint32_t holes  = usable > live_sectors ? usable - live_sectors : 0;

    out->live_chunks   = live_chunks;
    out->live_sectors  = live_sectors;
    out->total_sectors = span;
    out->hole_sectors  = holes;
    out->fragmentation = usable ? (float)holes / (float)usable : 0.0f;
}
