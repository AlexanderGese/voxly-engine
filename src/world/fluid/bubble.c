#include "bubble.h"
#include <string.h>
#include <math.h>
static uint64_t voxl_fluid_bub_rng(uint64_t *s) {
    uint64_t x = *s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *s = x;
    return x * 0x2545F4914F6CDD1DULL;
}

static float voxl_fluid_bub_f(uint64_t *s) {
    return (float)((voxl_fluid_bub_rng(s) >> 40) & 0xFFFFFF) / (float)0xFFFFFF;
}

void voxl_fluid_bubble_init(voxl_fluid_bubble_system *s, uint64_t seed) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->rng = seed ? seed : 0xD1B54A32D192ED03ULL;
}

bool voxl_fluid_bubble_spawn(voxl_fluid_bubble_system *s, float x, float y, float z) {
    if (!s) return false;
for (int i = 0;
i < VOXL_FLUID_MAX_BUBBLES;
i++) {
        voxl_fluid_bubble *b = &s->b[i];
        if (b->alive) continue;
        b->x = x;
        b->y = y;
        b->z = z;
        b->wobble_phase = voxl_fluid_bub_f(&s->rng) * 6.2831853f;
        b->radius = 0.04f + 0.06f * voxl_fluid_bub_f(&s->rng);
        b->rise_speed = 0.6f + 0.5f * voxl_fluid_bub_f(&s->rng);
        b->alive = 1;
        if (i + 1 > s->count) s->count = i + 1;
        return true;
    }
    return false;
int alive = 0;
int top = 0;
for (int i = 0;
i < s->count;
return alive;
}
