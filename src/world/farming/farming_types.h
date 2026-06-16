#ifndef WORLD_FARMING_TYPES_H
#define WORLD_FARMING_TYPES_H

#include <stdint.h>

// shared types for the farming subsystem. crops grow on hydrated farmland
// through a handful of discrete stages, then can be harvested for yields.
// this header has no logic, just the vocabulary everyone else speaks.

// crop species. wheat is the boring baseline; the rest tweak the curve.
typedef enum {
    FARMING_CROP_NONE = 0,
    FARMING_CROP_WHEAT,
    FARMING_CROP_CARROT,
    FARMING_CROP_POTATO,
    FARMING_CROP_BEETROOT,
    FARMING_CROP_MELON_STEM,   // stem grows, then spawns a fruit block beside it
    FARMING_CROP_PUMPKIN_STEM,
    FARMING_CROP_COUNT
} farming_crop_kind;

// every crop runs through [0 .. max_stage]. max_stage is per-species, so we
// keep a generous ceiling here and clamp per def.
#define FARMING_MAX_STAGES   8

// hydration is stored 0..7 like a wetness counter. 0 = bone dry farmland that
// will eventually revert to dirt, 7 = freshly watered. mirrors the block-light
// nibble convention used elsewhere so it packs nicely.
#define FARMING_HYDRATION_MAX   7

// how far (manhattan-ish, actually chebyshev on xz) we look for water to keep a
// tile hydrated. classic 4-block radius.
#define FARMING_WATER_RADIUS    4

// a single crop instance sitting on top of a farmland tile. world coords are
// the block the plant occupies (one above its farmland).
typedef struct {
    int      wx, wy, wz;
    uint8_t  kind;         // farming_crop_kind
    uint8_t  stage;        // 0..def->max_stage
    uint8_t  flags;        // FARMING_CROP_F_*
    uint8_t  _pad;
    float    growth_accum; // fractional progress toward the next stage
    uint32_t planted_tick; // for age-based bonuses / debug
} farming_crop;

enum {
    FARMING_CROP_F_ALIVE   = 1 << 0,
    FARMING_CROP_F_MATURE  = 1 << 1,  // reached max stage, harvestable
    FARMING_CROP_F_WILTED  = 1 << 2,  // lost its farmland, decaying
    FARMING_CROP_F_FRUITED = 1 << 3,  // stem already spawned its fruit
};

// a farmland tile. these outlive crops: you till dirt into farmland, it holds
// moisture, and a crop may or may not sit on top.
typedef struct {
    int     wx, wy, wz;
    uint8_t hydration;     // 0..FARMING_HYDRATION_MAX
    uint8_t has_crop;      // there is a live crop one block above
    uint8_t trample;       // accumulated trample damage, reverts to dirt at cap
    uint8_t _pad;
    float   dry_timer;     // seconds since last seen water; drives drying
} farming_tile;

// what a harvest produced. seeds are returned alongside the food yield so the
// player can replant. fruit blocks (melon/pumpkin) report via block_drop.
typedef struct {
    int produce_count;  // edible item count
    int seed_count;     // seeds returned
    int block_drop;     // a block_id to drop, or 0 for none (the melon/pumpkin)
    int xp;             // tiny xp blip, because every engine has one
} farming_yield;

#endif
