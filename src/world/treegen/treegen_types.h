#ifndef WORLD_TREEGEN_TYPES_H
#define WORLD_TREEGEN_TYPES_H
#include <stdint.h>
#include "../block.h"
// vegetation generator. l-system trees, bushes, grass scatter. like structgen
// this stays chunk-free and render-free: we grow plants into a voxel buffer and
// the deco pass hands the buffer back to the worldgen driver to stamp. keeps the
// link clean and lets us fuzz the grammar on its own without spinning a world.
// which species a slot resolved to. the grammar + tunables hang off this.
typedef enum {
    TREEGEN_NONE = 0,
    TREEGEN_OAK,        // round broadleaf, forking trunk
    TREEGEN_PINE,       // conifer, central spire + tiered whorls
    TREEGEN_BIRCH,      // tall thin, sparse canopy
    TREEGEN_PALM,       // bare trunk, fan crown
    TREEGEN_BUSH,       // no real trunk, just a leaf clump
    TREEGEN_KIND_COUNT
} treegen_kind;
typedef enum {
    TG_SYM_FWD = 0,     // 'F' draw forward one segment, leave wood
    TG_SYM_MOVE,        // 'f' move forward, no wood (used by palms)
    TG_SYM_PUSH,        // '['  push turtle state
    TG_SYM_POP,         // ']'  pop turtle state (leaves come from explicit L)
    TG_SYM_YAW_L,       // '+'  yaw left
    TG_SYM_YAW_R,       // '-'  yaw right
    TG_SYM_PITCH_D,     // '&'  pitch down
    TG_SYM_PITCH_U,     // '^'  pitch up
    TG_SYM_ROLL_L,      // '\\' roll left
    TG_SYM_ROLL_R,      // '/'  roll right
    TG_SYM_LEAF,        // 'L'  force a leaf cluster here
    TG_SYM_SHRINK,      // '!'  shrink segment radius/length one step
    TG_SYM_COUNT
} treegen_sym;
typedef struct {
    int x, y, z;
    block_id id;
} treegen_voxel;
typedef struct {
    block_id wood;
    block_id leaf;
} treegen_palette;
typedef struct {
    treegen_kind kind;
    treegen_palette pal;

    int   iterations;       // l-system rewrite depth
    int   seg_len;          // blocks per 'F' segment at full size
    float yaw_deg;          // turn angle for +/-
    float pitch_deg;        // pitch angle for &/^
    float roll_deg;         // roll angle for \//
    float angle_jitter;     // random wobble added to every turn, degrees

    int   trunk_radius;     // wood thickness at the base, in blocks
    int   leaf_radius;      // leaf cluster radius dropped at branch tips
    int   leaf_density;     // 0..100, odds a candidate leaf cell fills
    int   min_height;       // dont bother below this (sapling)
    int   max_height;       // clamp so a bad roll cant scrape the sky
} treegen_species;
treegen_species treegen_species_get(treegen_kind kind);
#endif
