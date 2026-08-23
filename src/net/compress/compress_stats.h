#ifndef NET_COMPRESS_STATS_H
#define NET_COMPRESS_STATS_H

// running stats for the compressor. handy when tuning the window / chain
// length or deciding if a stage is even worth keeping. the net send path
// feeds every encoded chunk through compress_stats_record.

#include <stddef.h>
#include <stdint.h>

#include "compress.h"

typedef struct {
    uint64_t chunks;          // chunks encoded
    uint64_t raw_bytes;       // total input bytes
    uint64_t packed_bytes;    // total output bytes
    uint64_t stage_hits[COMPRESS_M_COUNT]; // how often each stage was kept
    size_t   best_ratio_num;  // smallest output for a record (for min ratio)
    size_t   best_ratio_den;
    size_t   worst_ratio_num; // largest output (for max ratio)
    size_t   worst_ratio_den;
} compress_stats;

void  compress_stats_reset(compress_stats *s);

// record one encoded blob. `methods`/`method_count` come from the container.
void  compress_stats_record(compress_stats *s,
                            size_t raw_len, size_t packed_len,
                            const uint8_t *methods, int method_count);

// overall packed/raw ratio in 0..1. 1.0 means no savings at all.
float compress_stats_ratio(const compress_stats *s);

// dump a one-line summary via the log. nothing fancy.
void  compress_stats_dump(const compress_stats *s);

#endif
