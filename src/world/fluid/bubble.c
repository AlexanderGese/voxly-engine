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
    for (int i = 0; i < VOXL_FLUID_MAX_BUBBLES; i++) {
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
    return false;   // all slots busy
}

int voxl_fluid_bubble_emit(voxl_fluid_bubble_system *s,
                           float x, float y, float z, float rate, float dt) {
    if (!s || rate <= 0.0f) return 0;
    s->emit_acc += rate * dt;
    int spawned = 0;
    while (s->emit_acc >= 1.0f) {
        s->emit_acc -= 1.0f;
        // jitter the origin a touch so they dont stack in a line
        float jx = (voxl_fluid_bub_f(&s->rng) - 0.5f) * 0.3f;
        float jz = (voxl_fluid_bub_f(&s->rng) - 0.5f) * 0.3f;
        if (voxl_fluid_bubble_spawn(s, x + jx, y, z + jz)) spawned++;
        else break;
    }
    return spawned;
}

int voxl_fluid_bubble_update(voxl_fluid_bubble_system *s, float surface_y, float dt) {
    if (!s) return 0;
    int alive = 0;
    int top = 0;
    for (int i = 0; i < s->count; i++) {
        voxl_fluid_bubble *b = &s->b[i];
        if (!b->alive) continue;
        b->y += b->rise_speed * dt;
        // sideways wobble via the phase, bigger bubbles wobble more
        b->wobble_phase += dt * 4.0f;
        float w = sinf(b->wobble_phase) * b->radius;
        b->x += w * dt;
        b->z += cosf(b->wobble_phase * 0.7f) * b->radius * dt;
        if (b->y >= surface_y) {
            b->alive = 0;   // popped at the surface
            continue;
        }
        alive++;
        top = i + 1;
    }
    s->count = top;
    return alive;
}
