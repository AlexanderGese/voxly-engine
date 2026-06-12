#ifndef WORLD_SAVE_V2_H
#define WORLD_SAVE_V2_H
#include "chunk.h"
// newer disk format with run length encoded blocks and a world metadata
// sidecar. the older format lives in save.c for backwards load.
//
// file header:
// u32 magic 'VOX2'
// u32 version (2)
// i32 cx cz
// u32 num_runs
// runs: u8 block_id, u16 length
int  save_v2_write_chunk(const chunk *c, const char *path);
int  save_v2_read_chunk(chunk *c, const char *path);
int  save_v2_write_meta(unsigned seed, float day_hour);
int  save_v2_read_meta(unsigned *seed, float *day_hour);
#endif
