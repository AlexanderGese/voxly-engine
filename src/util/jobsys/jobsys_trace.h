#ifndef UTIL_JOBSYS_TRACE_H
#define UTIL_JOBSYS_TRACE_H
#include <stdint.h>
#include "jobsys_types.h"
#define JOBSYS_TRACE_RING  256
typedef struct {
    uint64_t start_us;     // when the job began
    uint32_t dur_us;       // how long it ran (capped at ~4s, plenty)
    uint16_t prio;         // jobsys_prio it carried
    uint16_t stolen;       // 1 if this worker stole the job, 0 if local
} jobsys_trace_sample;
#endif
