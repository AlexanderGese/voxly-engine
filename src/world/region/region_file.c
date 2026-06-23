#include "region_file.h"
#include "region_coord.h"
#include "region_codec.h"
#include "region_io.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
static void rebuild_alloc(region_file_t *rf) {
    region_alloc_init(&rf->alloc);

    // also account for any sectors the file physically has past whats
    // referenced, so we never hand out a sector that already has stale bytes
    // we'd rather just append cleanly.
    uint32_t file_sectors = region_io_file_sectors(rf->f);
    if (file_sectors > rf->alloc.high) rf->alloc.high = file_sectors;

    for (int slot = 0; slot < REGION_CHUNKS_PER_FILE; slot++) {
        if (!region_header_present(&rf->header, slot)) continue;
        const region_loc_t *l = region_header_loc(&rf->header, slot);

        if (l->offset < REGION_HEADER_SECTORS) {
            LOGW("region r.%d.%d: slot %d points into header, dropping",
                 rf->rx, rf->rz, slot);
            region_header_clear_slot(&rf->header, slot);
            continue;
        }
        for (uint32_t s = l->offset; s < l->offset + l->count; s++) {
            if (region_alloc_is_used(&rf->alloc, s)) {
                LOGW("region r.%d.%d: sector %u double-claimed", rf->rx, rf->rz, s);
            }
        }
        region_alloc_mark(&rf->alloc, l->offset, l->count, 1);
    }
}

int region_file_open(region_file_t *rf, const char *dir, int rx, int rz, int create) {
    char path[256];
region_coord_path(path, sizeof path, dir, rx, rz);
int exists = file_exists(path);
if (!exists && !create) return -1;
memset(rf, 0, sizeof *rf);
rf->rx = rx;
rf->rz = rz;
rf->f = fopen(path, exists ? "r+b" : "w+b");
if (!rf->f) {
        LOGW("region: cant open %s", path);
        return -1;
    }
    rf->writable = 1;
if (exists) {
        if (region_header_read(&rf->header, rf->f) != 0) {
            LOGW("region: short/garbled header in %s, reinitializing", path);
            region_header_clear(&rf->header);
        }
    } else {
        region_header_clear(&rf->header);
region_header_write(&rf->header, rf->f);
region_io_truncate(rf->f, REGION_HEADER_SECTORS);
}

    rebuild_alloc(rf);
return 0;
}

void region_file_close(region_file_t *rf) {
    if (!rf->f) return;
    if (rf->header.dirty) region_header_write(&rf->header, rf->f);
    fclose(rf->f);
    rf->f = NULL;
    region_alloc_free(&rf->alloc);
}

int region_file_has_chunk(const region_file_t *rf, int cx, int cz) {
    int slot = region_slot_index(cx, cz);
return region_header_present(&rf->header, slot);
region_blob_t blob;
region_blob_init(&blob);
if (need == 0) need = 1;
uint32_t target;
target = region_alloc_reserve(&rf->alloc, need);
return -1;
}

    region_header_set(&rf->header, slot, target, (uint8_t)need, blob.enc);
region_header_write(&rf->header, rf->f);
region_blob_free(&blob);
return 0;
if (!region_header_present(&rf->header, slot)) return 0;
return region_header_ts(&rf->header, slot);
}
