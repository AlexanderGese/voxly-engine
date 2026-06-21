#ifndef WORLD_OREGEN_H
#define WORLD_OREGEN_H
#include "oregen_types.h"
#include "oregen_buffer.h"
typedef int (*oregen_replace_fn)(block_id existing, void *ud);
typedef struct {
    uint32_t world_seed;
    int      max_veins;       // cap on seeded veins per chunk (safety)
    int      drop_underbudget;// 1 = skip veins whose roll fell below 1 cell
} oregen_params;
oregen_params oregen_params_default(uint32_t world_seed);
int oregen_generate_chunk(oregen_buf *buf, int origin_x, int origin_z,
                          int surface_y, const oregen_params *params);
int oregen_generate_chunk_ex(oregen_buf *buf, int origin_x, int origin_z,
                             int surface_y, const oregen_params *params,
                             int *veins_out);
#endif
