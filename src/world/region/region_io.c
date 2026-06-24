#include "region_io.h"
#include "../../util/log.h"

#include <string.h>

uint32_t region_io_sectors_for(size_t len) {
    return (uint32_t)((len + REGION_SECTOR_BYTES - 1) / REGION_SECTOR_BYTES);
}

static long sector_off(uint32_t sector) {
    return (long)sector * REGION_SECTOR_BYTES;
}

int region_io_read_sectors(FILE *f, uint32_t sector, uint32_t count, void *buf) {
    if (fseek(f, sector_off(sector), SEEK_SET) != 0) return -1;
    size_t want = (size_t)count * REGION_SECTOR_BYTES;
    size_t got  = fread(buf, 1, want, f);
    if (got != want) {
        // tail of file might be short if the last sector wasnt fully padded.
        // zero the remainder so the codec sees clean bytes past total_len.
        memset((char*)buf + got, 0, want - got);
    }
    return 0;
}

int region_io_write_payload(FILE *f, uint32_t sector, const void *data, size_t len) {
    if (fseek(f, sector_off(sector), SEEK_SET) != 0) return -1;
    if (len && fwrite(data, 1, len, f) != len) return -1;

    // pad the rest of the final sector with zeros
    uint32_t sectors = region_io_sectors_for(len);
    size_t   padded  = (size_t)sectors * REGION_SECTOR_BYTES;
    static const uint8_t zeros[256] = {0};
    size_t pad = padded - len;
    while (pad > 0) {
        size_t chunk = pad < sizeof zeros ? pad : sizeof zeros;
        if (fwrite(zeros, 1, chunk, f) != chunk) return -1;
        pad -= chunk;
    }
    fflush(f);
    return 0;
}

int region_io_truncate(FILE *f, uint32_t sectors) {
    long want = sector_off(sectors);
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    long cur = ftell(f);
    if (cur >= want) return 0;

    // grow by appending zeros. portable, no ftruncate dependency.
    static const uint8_t zeros[256] = {0};
    long pad = want - cur;
    while (pad > 0) {
        size_t chunk = (size_t)(pad < (long)sizeof zeros ? pad : (long)sizeof zeros);
        if (fwrite(zeros, 1, chunk, f) != chunk) return -1;
        pad -= chunk;
    }
    fflush(f);
    return 0;
}

uint32_t region_io_file_sectors(FILE *f) {
    if (fseek(f, 0, SEEK_END) != 0) return 0;
    long sz = ftell(f);
    if (sz < 0) return 0;
    return (uint32_t)(sz / REGION_SECTOR_BYTES);
}
