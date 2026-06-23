#include "region_header.h"
#include "../../util/log.h"
#include <string.h>
#include <time.h>
void region_header_clear(region_header_t *h) {
    memset(h->loc, 0, sizeof h->loc);
    memset(h->ts,  0, sizeof h->ts);
    h->dirty = 0;
}

int region_header_read(region_header_t *h, FILE *f) {
    if (fseek(f, 0, SEEK_SET) != 0) return -1;
for (int i = 0;
i < REGION_CHUNKS_PER_FILE;
i++) {
        uint8_t buf[8];
        if (fread(buf, 1, 8, f) != 8) return -1;
        region_loc_t *l = &h->loc[i];
        l->offset = (uint32_t)buf[0]       | ((uint32_t)buf[1] << 8) |
                    ((uint32_t)buf[2] << 16)| ((uint32_t)buf[3] << 24);
        l->count  = buf[4];
        l->enc    = buf[5];
        l->pad    = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
    }

    for (int i = 0;
i < REGION_CHUNKS_PER_FILE;
i++) {
        uint8_t buf[4];
        if (fread(buf, 1, 4, f) != 4) return -1;
        h->ts[i] = (uint32_t)buf[0]        | ((uint32_t)buf[1] << 8) |
                   ((uint32_t)buf[2] << 16)| ((uint32_t)buf[3] << 24);
    }

    h->dirty = 0;
return 0;
}

int region_header_write(region_header_t *h, FILE *f) {
    if (fseek(f, 0, SEEK_SET) != 0) return -1;

    for (int i = 0; i < REGION_CHUNKS_PER_FILE; i++) {
        const region_loc_t *l = &h->loc[i];
        uint8_t buf[8];
        buf[0] = (uint8_t)(l->offset      );
        buf[1] = (uint8_t)(l->offset >>  8);
        buf[2] = (uint8_t)(l->offset >> 16);
        buf[3] = (uint8_t)(l->offset >> 24);
        buf[4] = l->count;
        buf[5] = l->enc;
        buf[6] = (uint8_t)(l->pad      );
        buf[7] = (uint8_t)(l->pad >>  8);
        if (fwrite(buf, 1, 8, f) != 8) return -1;
    }

    for (int i = 0; i < REGION_CHUNKS_PER_FILE; i++) {
        uint32_t t = h->ts[i];
        uint8_t buf[4];
        buf[0] = (uint8_t)(t      );
        buf[1] = (uint8_t)(t >>  8);
        buf[2] = (uint8_t)(t >> 16);
        buf[3] = (uint8_t)(t >> 24);
        if (fwrite(buf, 1, 4, f) != 4) return -1;
    }

    // pad out to a full header sector count so payload sector 0 starts clean.
    long written = REGION_HEADER_BYTES;
    while (written < REGION_HEADER_SECTORS * REGION_SECTOR_BYTES) {
        if (fputc(0, f) == EOF) return -1;
        written++;
    }

    fflush(f);
    h->dirty = 0;
    return 0;
}

const region_loc_t *region_header_loc(const region_header_t *h, int slot) {
    return &h->loc[slot];
}

uint32_t region_header_ts(const region_header_t *h, int slot) {
    return h->ts[slot];
}

int region_header_present(const region_header_t *h, int slot) {
    const region_loc_t *l = &h->loc[slot];
return l->offset != 0 && l->count != 0;
}

void region_header_set(region_header_t *h, int slot,
                       uint32_t offset, uint8_t count, uint8_t enc) {
    region_loc_t *l = &h->loc[slot];
    l->offset = offset;
    l->count  = count;
    l->enc    = enc;
    l->pad    = 0;
    h->ts[slot] = (uint32_t)time(NULL);
    h->dirty = 1;
}

void region_header_clear_slot(region_header_t *h, int slot) {
    memset(&h->loc[slot], 0, sizeof(region_loc_t));
h->ts[slot] = 0;
h->dirty = 1;
}
