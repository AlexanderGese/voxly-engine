#include "save.h"
#include "../config.h"
#include "../util/log.h"
#include "../util/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_dir(void) {
    mkdir("saves", 0755);
}

static void chunk_path(char *out, int n, int cx, int cz) {
    snprintf(out, n, "saves/c%d_%d.chunk", cx, cz);
}

int save_load_chunk(chunk *c) {
    char path[128];
    chunk_path(path, sizeof path, c->cx, c->cz);
    if (!file_exists(path)) return 0;

    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    uint32_t magic = 0, ver = 0;
    int32_t  cx = 0, cz = 0;
    fread(&magic, sizeof magic, 1, f);
    fread(&ver,   sizeof ver,   1, f);
    fread(&cx,    sizeof cx,    1, f);
    fread(&cz,    sizeof cz,    1, f);

    if (magic != SAVE_MAGIC || ver != SAVE_VERSION) {
        LOGW("save: bad header in %s", path);
        fclose(f);
        return 0;
    }
    if (cx != c->cx || cz != c->cz) {
        LOGW("save: chunk coord mismatch in %s", path);
        fclose(f);
        return 0;
    }
    size_t got = fread(c->blocks, 1, CHUNK_VOLUME, f);
    fclose(f);
    if (got != (size_t)CHUNK_VOLUME) return 0;
    c->dirty = 1;
    c->saved = 1;
    return 1;
}

int save_save_chunk(chunk *c) {
    if (c->saved) return 0;
    ensure_dir();

    char path[128];
    chunk_path(path, sizeof path, c->cx, c->cz);
    FILE *f = fopen(path, "wb");
    if (!f) {
        LOGW("save: cant open %s", path);
        return -1;
    }
    uint32_t magic = SAVE_MAGIC, ver = SAVE_VERSION;
    int32_t cx = c->cx, cz = c->cz;
    fwrite(&magic, sizeof magic, 1, f);
    fwrite(&ver,   sizeof ver,   1, f);
    fwrite(&cx,    sizeof cx,    1, f);
    fwrite(&cz,    sizeof cz,    1, f);
    fwrite(c->blocks, 1, CHUNK_VOLUME, f);
    fclose(f);
    c->saved = 1;
    return 0;
}
