#include "octree_morton.h"

// the classic magic-number bit spread. takes 21 low bits and smears them out
// so every 3rd bit is occupied. i looked this up the first 5 times, now i just
// trust past-me that the constants are right.
static uint64_t spread3(uint32_t v) {
    uint64_t x = v & OCTREE_MORTON_MASK;
    x = (x | (x << 32)) & 0x1f00000000ffffULL;
    x = (x | (x << 16)) & 0x1f0000ff0000ffULL;
    x = (x | (x << 8))  & 0x100f00f00f00f00fULL;
    x = (x | (x << 4))  & 0x10c30c30c30c30c3ULL;
    x = (x | (x << 2))  & 0x1249249249249249ULL;
    return x;
}

// inverse of spread3. pulls every 3rd bit back down into a dense 21 bit value.
static uint32_t compact3(uint64_t x) {
    x &= 0x1249249249249249ULL;
    x = (x | (x >> 2))  & 0x10c30c30c30c30c3ULL;
    x = (x | (x >> 4))  & 0x100f00f00f00f00fULL;
    x = (x | (x >> 8))  & 0x1f0000ff0000ffULL;
    x = (x | (x >> 16)) & 0x1f00000000ffffULL;
    x = (x | (x >> 32)) & OCTREE_MORTON_MASK;
    return (uint32_t)x;
}

uint64_t octree_morton_encode(uint32_t x, uint32_t y, uint32_t z) {
    return spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
}

void octree_morton_decode(uint64_t code, uint32_t *x, uint32_t *y, uint32_t *z) {
    if (x) *x = compact3(code);
    if (y) *y = compact3(code >> 1);
    if (z) *z = compact3(code >> 2);
}

int octree_morton_child(uint64_t code, int level) {
    // level 0 is the most significant triplet. shift so the wanted triplet
    // lands at the bottom, then mask 3 bits.
    int shift = (OCTREE_MORTON_BITS - 1 - level) * 3;
    if (shift < 0) shift = 0;
    return (int)((code >> shift) & 0x7u);
}
