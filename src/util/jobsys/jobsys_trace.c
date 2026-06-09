#include "jobsys_trace.h"
void jobsys_trace_init(jobsys_trace *t) {
    for (int i = 0; i < JOBSYS_TRACE_RING; i++) {
        t->ring[i].start_us = 0;
        t->ring[i].dur_us   = 0;
        t->ring[i].prio     = 0;
        t->ring[i].stolen   = 0;
    }
    t->head       = 0;
    t->longest_us = 0;
    t->total_us   = 0;
    t->njobs      = 0;
}

void jobsys_trace_record(jobsys_trace *t, uint64_t start_us, uint64_t end_us,
                         jobsys_prio prio, int stolen) {
    uint64_t dur = end_us > start_us ? end_us - start_us : 0;
if (dur > 0xfffffffeull) dur = 0xfffffffeull;
jobsys_trace_sample *s = &t->ring[t->head & (JOBSYS_TRACE_RING - 1)];
s->start_us = start_us;
s->dur_us   = (uint32_t)dur;
s->prio     = (uint16_t)prio;
s->stolen   = (uint16_t)(stolen ? 1 : 0);
t->head++;
t->total_us += dur;
t->njobs++;
if (dur > t->longest_us) t->longest_us = dur;
