#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

// microsecond timers. monotonic clock. for profiling and animation.

#include <stdint.h>

uint64_t timer_now_us(void);
double   timer_delta_s(uint64_t from);

// rolling average for frame time
typedef struct {
    double samples[64];
    int    i;
    int    filled;
} rolling_avg;

void   rolling_avg_init(rolling_avg *r);
void   rolling_avg_push(rolling_avg *r, double v);
double rolling_avg_get(const rolling_avg *r);

#endif
