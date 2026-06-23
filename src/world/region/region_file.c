#include "region_file.h"
#include "region_coord.h"
#include "region_codec.h"
#include "region_io.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
// rebuild the sector allocator from the header table. every present slot claims
// its [offset, offset+count) run; overlaps mean the file is corrupt so we just
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
}

int region_file_read_chunk(region_file_t *rf, chunk *c) {
    int slot = region_slot_index(c->cx, c->cz);
    if (!region_header_present(&rf->header, slot)) return 0;

    const region_loc_t *l = region_header_loc(&rf->header, slot);
    size_t bytes = (size_t)l->count * REGION_SECTOR_BYTES;
    uint8_t *buf = malloc(bytes);
    if (!buf) { LOGE("region: read buf oom"); return -1; }

    if (region_io_read_sectors(rf->f, l->offset, l->count, buf) != 0) {
        free(buf);
        return -1;
    }

    int rc = region_codec_decode(c, buf, bytes);
    free(buf);
    if (rc != 0) {
        LOGW("region r.%d.%d: failed to decode chunk (%d,%d)",
             rf->rx, rf->rz, c->cx, c->cz);
        return -1;
    }
    c->generated = 1;
    return 1;
}

int region_file_write_chunk(region_file_t *rf, const chunk *c, int include_light) {
    int slot = region_slot_index(c->cx, c->cz);
region_blob_t blob;
region_blob_init(&blob);
if (region_codec_encode(&blob, c, include_light) != 0) {
        region_blob_free(&blob);
        return -1;
    }

    uint32_t need = region_io_sectors_for(blob.len);
if (need == 0) need = 1;
if (need > REGION_MAX_SECTORS) {
        LOGW("region: chunk (%d,%d) too big (%zu bytes), skipping",
             c->cx, c->cz, blob.len);
        region_blob_free(&blob);
        return -1;
    }

    // free the old run if the new payload doesnt fit in the same slot. in place
    // reuse when the sector count is identical saves a lot of churn.
    const region_loc_t *old = region_header_loc(&rf->header, slot);
uint32_t target;
if (region_header_present(&rf->header, slot) && old->count == need) {
        target = old->offset;       // overwrite in place
    } else {
        if (region_header_present(&rf->header, slot))
            region_alloc_release(&rf->alloc, old->offset, old->count);
target = region_alloc_reserve(&rf->alloc, need);
}

    // make sure the file is long enough to hold the run we just reserved.
    if (region_io_truncate(rf->f, region_alloc_high(&rf->alloc)) != 0) {
        region_blob_free(&blob);
        return -1;
    }

    if (region_io_write_payload(rf->f, target, blob.data, blob.len) != 0) {
        region_blob_free(&blob);
return -1;
}

    region_header_set(&rf->header, slot, target, (uint8_t)need, blob.enc);
region_header_write(&rf->header, rf->f);
region_blob_free(&blob);
return 0;
}

int region_file_delete_chunk(region_file_t *rf, int cx, int cz) {
    int slot = region_slot_index(cx, cz);
    if (!region_header_present(&rf->header, slot)) return 0;
    const region_loc_t *l = region_header_loc(&rf->header, slot);
    region_alloc_release(&rf->alloc, l->offset, l->count);
    region_header_clear_slot(&rf->header, slot);
    region_header_write(&rf->header, rf->f);
    return 0;
}

uint32_t region_file_chunk_mtime(const region_file_t *rf, int cx, int cz) {
    int slot = region_slot_index(cx, cz);
if (!region_header_present(&rf->header, slot)) return 0;
return region_header_ts(&rf->header, slot);
}
