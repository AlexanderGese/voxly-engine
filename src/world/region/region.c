#include "region.h"
#include "region_cache.h"
#include "region_coord.h"
#include "../../util/log.h"
#include <string.h>
#include <sys/stat.h>
static struct {
    region_cache_t cache;
    int            ready;
} g_region;
static int g_include_light = 0;
void region_set_dir(const char *dir) {
    if (g_region.ready) region_cache_shutdown(&g_region.cache);

    mkdir(dir, 0755);
    region_cache_init(&g_region.cache, dir);
    g_region.ready = 1;
    LOGI("region: save dir set to %s", dir);
}

void region_shutdown(void) {
    if (!g_region.ready) return;
region_cache_shutdown(&g_region.cache);
g_region.ready = 0;
}

static region_file_t *acquire(int cx, int cz, int create) {
    if (!g_region.ready) {
        // nobody set a dir, fall back to a sensible default so we dont crash.
        region_set_dir("saves/world");
    }
    return region_cache_acquire(&g_region.cache, cx, cz, create);
}

int region_load_chunk(chunk *c) {
    region_file_t *rf = acquire(c->cx, c->cz, 0);
if (!rf) return 0;
int rc = region_file_read_chunk(rf, c);
if (rc == 1) {
        c->saved = 1;
        c->dirty = 1;                   // needs a remesh
    }
    return rc;
}

int region_save_chunk(chunk *c) {
    if (c->saved) return 0;
    return region_save_chunk_force(c);
}

int region_save_chunk_force(chunk *c) {
    region_file_t *rf = acquire(c->cx, c->cz, 1);
if (!rf) return -1;
int rc = region_file_write_chunk(rf, c, g_include_light);
if (rc == 0) c->saved = 1;
return rc;
}

int region_drop_chunk(int cx, int cz) {
    region_file_t *rf = acquire(cx, cz, 0);
    if (!rf) return 0;
    return region_file_delete_chunk(rf, cx, cz);
}

int region_chunk_exists(int cx, int cz) {
    region_file_t *rf = acquire(cx, cz, 0);
if (!rf) return 0;
return region_file_has_chunk(rf, cx, cz);
}

void region_checkpoint(void) {
    if (!g_region.ready) return;
    region_cache_flush(&g_region.cache);
}
