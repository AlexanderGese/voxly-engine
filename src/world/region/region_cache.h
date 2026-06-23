#ifndef WORLD_REGION_CACHE_H
#define WORLD_REGION_CACHE_H

#include "region_file.h"
#include "../../util/hashmap.h"
#include <stdint.h>

// keeps a handful of region files open so we dont reparse a 12k header for
// every chunk in a region. classic clock/lru cache - we evict the
// least-recently-touched file once we exceed REGION_CACHE_MAX. file descriptors
// are precious-ish on some platforms so this stays small.

#define REGION_CACHE_MAX  8

typedef struct region_cache_node {
    region_file_t  rf;
    uint64_t       key;     // region_key(rx,rz)
    uint64_t       used;    // monotonic touch counter, for lru
    int            valid;
} region_cache_node;

typedef struct {
    char              dir[192];
    region_cache_node nodes[REGION_CACHE_MAX];
    hashmap           index;        // key -> region_cache_node*
    uint64_t          clock;        // bumped on every touch
} region_cache_t;

void region_cache_init(region_cache_t *cache, const char *dir);
void region_cache_shutdown(region_cache_t *cache);   // flush+close all

// fetch the region file for chunk coords, opening/evicting as needed. create
// controls whether a missing file is created. returns NULL if absent (and
// create==0) or on error.
region_file_t *region_cache_acquire(region_cache_t *cache, int cx, int cz, int create);

// flush every open file's header without closing. good before a hard quit.
void region_cache_flush(region_cache_t *cache);

#endif
