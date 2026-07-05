#ifndef RENDER_DECALS_ATLAS_H
#define RENDER_DECALS_ATLAS_H

#include "decals_types.h"
#include "decals_config.h"
#include "../gl.h"

// the decal atlas. one gl texture laid out as a DECALS_ATLAS_TILES_X by
// DECALS_ATLAS_TILES_Y grid. each named region claims one or more contiguous
// tiles for its albedo, and optionally a second block of tiles for its normal
// map. we keep a name->region table so spawn sites can ask for "scorch" instead
// of memorising tile coordinates.

#define DECALS_ATLAS_NAME_LEN  24

typedef struct {
    char  name[DECALS_ATLAS_NAME_LEN];
    decals_atlas_region region;   // resolved uv rects
    uint16_t flags;               // copied onto decals spawned from this region
} decals_atlas_entry;

typedef struct {
    glid  tex;            // the gl texture, 0 until uploaded
    int   tiles_x, tiles_y;
    decals_atlas_entry entries[DECALS_ATLAS_MAX_REGIONS];
    int   count;
} decals_atlas;

// zero the table and set the grid size. does not touch gl.
void decals_atlas_init(decals_atlas *a);

// register a region from albedo tile coords (tx,ty) spanning (tw,th) tiles.
// if has_normal, the normal map block starts at (ntx,nty) same span. returns
// the new region index, or -1 if the table is full / name already taken.
int  decals_atlas_add(decals_atlas *a, const char *name,
                      int tx, int ty, int tw, int th,
                      int has_normal, int ntx, int nty, uint16_t flags);

// look up by name. returns index or -1.
int  decals_atlas_find(const decals_atlas *a, const char *name);

// fetch a resolved region by index. null on out-of-range.
const decals_atlas_region *decals_atlas_region_at(const decals_atlas *a, int idx);
uint16_t decals_atlas_flags_at(const decals_atlas *a, int idx);

// hand the atlas a gl texture id (created/loaded elsewhere). bind for drawing.
void decals_atlas_set_texture(decals_atlas *a, glid tex);
void decals_atlas_bind(const decals_atlas *a, int unit);

// register the handful of regions the engine ships with. convenience so the
// renderer doesnt have to spell out every footprint by hand.
void decals_atlas_load_defaults(decals_atlas *a);

#endif
