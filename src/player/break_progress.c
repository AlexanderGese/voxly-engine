#include "break_progress.h"

void break_progress_init(break_progress *b) {
    b->active = 0;
    b->progress = 0;
    b->break_time = 1.0f;
    b->x = b->y = b->z = 0;
}

void break_progress_begin(break_progress *b, int x, int y, int z, float t) {
    b->active = 1;
    b->x = x; b->y = y; b->z = z;
    b->progress = 0;
    b->break_time = t > 0.01f ? t : 0.5f;
}

int break_progress_tick(break_progress *b, int x, int y, int z, int holding, float dt) {
    if (!holding) { b->active = 0; b->progress = 0; return 0; }
    if (b->x != x || b->y != y || b->z != z) {
        break_progress_begin(b, x, y, z, b->break_time);
    }
    if (!b->active) break_progress_begin(b, x, y, z, b->break_time);
    b->progress += dt / b->break_time;
    if (b->progress >= 1.0f) {
        b->active = 0;
        b->progress = 0;
        return 1;
    }
    return 0;
}

int break_progress_phase(const break_progress *b) {
    if (!b->active) return 0;
    int p = (int)(b->progress * 10.0f);
    if (p < 1) p = 1;
    if (p > 10) p = 10;
    return p;
}

void break_progress_cancel(break_progress *b) {
    b->active = 0;
    b->progress = 0;
}
