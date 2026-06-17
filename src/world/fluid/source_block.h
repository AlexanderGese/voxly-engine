#ifndef WORLD_FLUID_SOURCE_BLOCK_H
#define WORLD_FLUID_SOURCE_BLOCK_H
#include "fluid_cell.h"
// source blocks. these are infinite fluid emitters (placed bucket, spring).
// we keep them in a little list and re-stamp them to full every step so the
// spread passes always have something to draw from.
#define VOXL_FLUID_MAX_SOURCES 64
typedef struct {
    int     x, y, z;
    uint8_t kind;     // VOXL_FLUID_WATER / LAVA
    uint8_t active;   // 0 = free slot
} voxl_fluid_source;
typedef struct voxl_fluid_source_set {
    voxl_fluid_source items[VOXL_FLUID_MAX_SOURCES];
    int count;
} voxl_fluid_source_set;
void voxl_fluid_sources_clear(voxl_fluid_source_set *s);
// add a source. returns its slot index, or -1 if full / already present.
int  voxl_fluid_source_add(voxl_fluid_source_set *s, int x, int y, int z, uint8_t kind);
bool voxl_fluid_source_remove(voxl_fluid_source_set *s, int x, int y, int z);
bool voxl_fluid_source_at(const voxl_fluid_source_set *s, int x, int y, int z);
void voxl_fluid_sources_apply(const voxl_fluid_source_set *s, voxl_fluid_grid *g);
int  voxl_fluid_sources_form(voxl_fluid_source_set *s, const voxl_fluid_grid *g);
#endif
