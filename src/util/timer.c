#include "timer.h"

#include <time.h>

uint64_t timer_now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

double timer_delta_s(uint64_t from) {
    uint64_t now = timer_now_us();
    return (double)(now - from) / 1e6;
}

void rolling_avg_init(rolling_avg *r) {
    for (int i = 0; i < 64; i++) r->samples[i] = 0;
    r->i = 0;
    r->filled = 0;
}

void rolling_avg_push(rolling_avg *r, double v) {
    r->samples[r->i] = v;
    r->i = (r->i + 1) & 63;
    if (!r->filled && r->i == 0) r->filled = 1;
}

double rolling_avg_get(const rolling_avg *r) {
    int n = r->filled ? 64 : r->i;
    if (n == 0) return 0;
    double s = 0;
    for (int i = 0; i < n; i++) s += r->samples[i];
    return s / (double)n;
}
