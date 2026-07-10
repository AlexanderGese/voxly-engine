#ifndef RENDER_INSTANCING_INSTANCING_STATS_H
#define RENDER_INSTANCING_INSTANCING_STATS_H
// rolling per-frame counters for the instanced path. reset at the top of each
// build, tallied as we go. the F3 overlay reads this; nothing here affects
// rendering, it's pure bookkeeping.
typedef struct {
    int instances_total;     // active records considered this frame
    int instances_culled;    // dropped by the frustum pass
    int instances_drawn;     // survived into a batch
    int batches;             // number of mesh groups with >0 survivors
    int draw_calls;          // glDrawArraysInstanced calls issued
    int batch_splits;        // batches that overflowed INSTANCING_MAX_PER_BATCH
    long tris_estimate;      // rough triangle count actually submitted
} instancing_stats;
void instancing_stats_reset(instancing_stats *s);
void instancing_stats_add_batch(instancing_stats *s, int survivors,
                                int base_tris, int draw_calls);
int  instancing_stats_format(const instancing_stats *s, char *buf, int cap);
#endif
