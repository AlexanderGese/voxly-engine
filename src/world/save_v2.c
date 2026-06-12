#include "save_v2.h"
#include "../util/log.h"
#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAGIC_V2 0x32584F56u    /* 'VOX2' */

static void ensure_dir(void) {
    mkdir("saves", 0755);
}

int save_v2_write_chunk(const chunk *c, const char *path) {
    ensure_dir();
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    uint32_t magic = MAGIC_V2;
    uint32_t ver = 2;
    int32_t cx = c->cx, cz = c->cz;
    fwrite(&magic, 4, 1, f);
    fwrite(&ver,   4, 1, f);
    fwrite(&cx,    4, 1, f);
    fwrite(&cz,    4, 1, f);

    // rle encode
    size_t n = CHUNK_VOLUME;
    uint32_t runs = 0;
    long runs_pos = ftell(f);
    fwrite(&runs, 4, 1, f);

    size_t i = 0;
    while (i < n) {
        uint8_t id = c->blocks[i];
        uint16_t len = 1;
        while (i + len < n && c->blocks[i + len] == id && len < 65535) len++;
        fwrite(&id,  1, 1, f);
        fwrite(&len, 2, 1, f);
        runs++;
        i += len;
    }

    fseek(f, runs_pos, SEEK_SET);
    fwrite(&runs, 4, 1, f);
    fclose(f);
    return 0;
}

int save_v2_read_chunk(chunk *c, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    uint32_t magic, ver;
    int32_t cx, cz;
    uint32_t runs;
    fread(&magic, 4, 1, f);
    fread(&ver,   4, 1, f);
    fread(&cx,    4, 1, f);
    fread(&cz,    4, 1, f);
    fread(&runs,  4, 1, f);
    if (magic != MAGIC_V2) { fclose(f); return -1; }
    if (cx != c->cx || cz != c->cz) { fclose(f); return -1; }

    size_t i = 0;
    for (uint32_t r = 0; r < runs; r++) {
        uint8_t id;
        uint16_t len;
        fread(&id,  1, 1, f);
        fread(&len, 2, 1, f);
        for (int k = 0; k < len && i < CHUNK_VOLUME; k++) c->blocks[i++] = id;
    }
    fclose(f);
    c->saved = 1;
    c->dirty = 1;
    return 0;
}

int save_v2_write_meta(unsigned seed, float day_hour) {
    ensure_dir();
    FILE *f = fopen("saves/world.meta", "wb");
    if (!f) return -1;
    fwrite(&seed, sizeof seed, 1, f);
    fwrite(&day_hour, sizeof day_hour, 1, f);
    fclose(f);
    return 0;
}

int save_v2_read_meta(unsigned *seed, float *day_hour) {
    FILE *f = fopen("saves/world.meta", "rb");
    if (!f) return -1;
    fread(seed, sizeof *seed, 1, f);
    fread(day_hour, sizeof *day_hour, 1, f);
    fclose(f);
    return 0;
}
