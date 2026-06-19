#ifndef WORLD_LIGHTPROP_TYPES_H
#define WORLD_LIGHTPROP_TYPES_H
#include "../world.h"
#include "../chunk.h"
#include "../block.h"
#include "../../config.h"
#include <stdint.h>
// incremental light propagation. the old lighting.c nukes and rebuilds a whole
// chunk on every edit which is fine until you place a torch in a cave and watch
// the frame hitch. this does it the right way: bfs flood from sources, and a
typedef enum {
    LP_BLOCK = 0,   // emitter light
    LP_SKY   = 1    // sky light
} lp_channel;
typedef struct {
    int32_t x, y, z;
    uint8_t level;   // for adds: the light here. for removes: the OLD light here.
} lp_node;
extern const int LP_DX[6];
extern const int LP_DY[6];
extern const int LP_DZ[6];
static inline int lp_y_in_range(int y) {
    return y >= 0 && y < CHUNK_SIZE_Y;
}

// how much a block attenuates light passing through it. opaque blocks don't
// transmit at all (handled by the caller skipping them);
transparent-but-tinted
// blocks (water, ice) eat an extra step. returns the cost ON TOP of the base 1.
int lp_attenuation(block_id id);
int lp_transmits(block_id id);
#endif
