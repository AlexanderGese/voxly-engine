#include "biome_table.h"

#include <stddef.h>

// envelope fields are: temp, humid, eros, weird, then the four axis weights.
// weights say how strongly each climate axis pulls toward this biome. zero
// weight == "dont care". tuned by hand, expect to keep nudging these.

static const biome_def g_biomes[BIOME_KIND_COUNT] = {
    [BIOME_KIND_OCEAN] = {
        "ocean", BIOME_KIND_OCEAN,
        { 0.50f, 0.70f, 0.95f, 0.50f,  0.4f, 0.4f, 1.6f, 0.0f },
        BLOCK_SAND, BLOCK_SAND, BLOCK_STONE, BLOCK_SAND,
        -14.0f, 4.0f, 3,
        0.0f, 0.0f, 0, 0x3F76E4, 0x3F76E4
    },
    [BIOME_KIND_BEACH] = {
        "beach", BIOME_KIND_BEACH,
        { 0.55f, 0.55f, 0.85f, 0.50f,  0.3f, 0.3f, 1.2f, 0.0f },
        BLOCK_SAND, BLOCK_SAND, BLOCK_STONE, BLOCK_SAND,
        1.0f, 2.0f, 2,
        0.02f, 0.05f, 0, 0x91BD59, 0x3F76E4
    },
    [BIOME_KIND_PLAINS] = {
        "plains", BIOME_KIND_PLAINS,
        { 0.55f, 0.45f, 0.55f, 0.50f,  0.8f, 0.9f, 0.5f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        4.0f, 6.0f, 4,
        0.04f, 0.6f, 0, 0x91BD59, 0x3F76E4
    },
    [BIOME_KIND_FOREST] = {
        "forest", BIOME_KIND_FOREST,
        { 0.52f, 0.62f, 0.50f, 0.50f,  0.7f, 1.0f, 0.4f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        6.0f, 9.0f, 4,
        0.18f, 0.5f, 0, 0x79C05A, 0x3F76E4
    },
    [BIOME_KIND_RAINFOREST] = {
        "rainforest", BIOME_KIND_RAINFOREST,
        { 0.80f, 0.88f, 0.45f, 0.50f,  1.2f, 1.4f, 0.4f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        8.0f, 12.0f, 4,
        0.32f, 0.7f, 0, 0x59C93C, 0x3FB0E4
    },
    [BIOME_KIND_SWAMP] = {
        "swamp", BIOME_KIND_SWAMP,
        { 0.62f, 0.82f, 0.78f, 0.50f,  0.6f, 1.3f, 0.7f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        0.0f, 3.0f, 3,
        0.10f, 0.4f, 0, 0x6A7039, 0x617B64
    },
    [BIOME_KIND_SAVANNA] = {
        "savanna", BIOME_KIND_SAVANNA,
        { 0.78f, 0.30f, 0.55f, 0.50f,  1.1f, 1.1f, 0.4f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        5.0f, 7.0f, 4,
        0.05f, 0.5f, 0, 0xBFB755, 0x3F76E4
    },
    [BIOME_KIND_DESERT] = {
        "desert", BIOME_KIND_DESERT,
        { 0.88f, 0.12f, 0.55f, 0.50f,  1.3f, 1.5f, 0.4f, 0.0f },
        BLOCK_SAND, BLOCK_SAND, BLOCK_STONE, BLOCK_SAND,
        5.0f, 8.0f, 5,
        0.005f, 0.05f, 0, 0xBFB755, 0x3F76E4
    },
    [BIOME_KIND_BADLANDS] = {
        "badlands", BIOME_KIND_BADLANDS,
        { 0.90f, 0.20f, 0.25f, 0.78f,  1.0f, 1.0f, 0.6f, 1.4f },
        BLOCK_SAND, BLOCK_SAND, BLOCK_STONE, BLOCK_SAND,
        12.0f, 22.0f, 6,
        0.0f, 0.1f, 0, 0x90814D, 0x4E7F81
    },
    [BIOME_KIND_TAIGA] = {
        "taiga", BIOME_KIND_TAIGA,
        { 0.28f, 0.55f, 0.50f, 0.50f,  1.2f, 0.7f, 0.4f, 0.0f },
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_DIRT,
        7.0f, 11.0f, 4,
        0.20f, 0.4f, 1, 0x86B783, 0x3D57D6
    },
    [BIOME_KIND_TUNDRA] = {
        "tundra", BIOME_KIND_TUNDRA,
        { 0.14f, 0.35f, 0.60f, 0.50f,  1.4f, 0.6f, 0.4f, 0.0f },
        BLOCK_SNOW, BLOCK_DIRT, BLOCK_STONE, BLOCK_ICE,
        4.0f, 5.0f, 4,
        0.01f, 0.2f, 1, 0x80B497, 0x3938C9
    },
    [BIOME_KIND_SNOWY_PEAKS] = {
        "snowy_peaks", BIOME_KIND_SNOWY_PEAKS,
        { 0.10f, 0.50f, 0.10f, 0.50f,  1.3f, 0.3f, 1.4f, 0.0f },
        BLOCK_SNOW, BLOCK_STONE, BLOCK_STONE, BLOCK_ICE,
        40.0f, 30.0f, 2,
        0.0f, 0.0f, 1, 0x80B497, 0x3938C9
    },
    [BIOME_KIND_STONE_PEAKS] = {
        "stone_peaks", BIOME_KIND_STONE_PEAKS,
        { 0.40f, 0.40f, 0.08f, 0.50f,  0.5f, 0.5f, 1.6f, 0.0f },
        BLOCK_STONE, BLOCK_STONE, BLOCK_STONE, BLOCK_COBBLE,
        44.0f, 34.0f, 1,
        0.0f, 0.0f, 0, 0x8AB689, 0x3F76E4
    },
};

const biome_def *biome_table_get(biome_kind kind) {
    if (kind < 0 || kind >= BIOME_KIND_COUNT) return &g_biomes[BIOME_KIND_PLAINS];
    return &g_biomes[kind];
}

int biome_table_count(void) {
    return BIOME_KIND_COUNT;
}

const char *biome_table_name(biome_kind kind) {
    if (kind < 0 || kind >= BIOME_KIND_COUNT) return "?";
    return g_biomes[kind].name;
}

const biome_def *biome_table_at(int index) {
    if (index < 0 || index >= BIOME_KIND_COUNT) return NULL;
    return &g_biomes[index];
}
