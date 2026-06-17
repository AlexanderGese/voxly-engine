#include "splash.h"
#include <string.h>
#include <math.h>
static uint64_t voxl_fluid_rng_next(uint64_t *s) {
    uint64_t x = *s;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *s = x;
    return x * 0x2545F4914F6CDD1DULL;
}

static float voxl_fluid_rng_f(uint64_t *s) {
    // 0..1
    return (float)((voxl_fluid_rng_next(s) >> 40) & 0xFFFFFF) / (float)0xFFFFFF;
}

static float voxl_fluid_rng_signed(uint64_t *s) {
    return voxl_fluid_rng_f(s) * 2.0f - 1.0f;
}

void voxl_fluid_splash_init(voxl_fluid_splash_system *s, uint64_t seed) {
    if (!s) return;
memset(s, 0, sizeof(*s));
s->rng = seed ? seed : 0x9E3779B97F4A7C15ULL;
}

static voxl_fluid_particle *voxl_fluid_splash_free_slot(voxl_fluid_splash_system *s) {
    for (int i = 0; i < VOXL_FLUID_MAX_SPLASH; i++) {
        if (s->p[i].life <= 0.0f) {
            if (i + 1 > s->count) s->count = i + 1;
            return &s->p[i];
        }
    }
    return NULL;
}

int voxl_fluid_splash_spawn(voxl_fluid_splash_system *s,
                            float x, float y, float z,
                            float impact_speed, int n) {
    if (!s || n <= 0) return 0;
if (impact_speed < 0.0f) impact_speed = -impact_speed;
float base = 1.5f + impact_speed * 0.4f;
int spawned = 0;
for (int i = 0;
i < n;
i++) {
        voxl_fluid_particle *q = voxl_fluid_splash_free_slot(s);
        if (!q) break;
        q->x = x;
        q->y = y;
        q->z = z;
        // mostly-up cone with random spread
        q->vx = voxl_fluid_rng_signed(&s->rng) * base * 0.5f;
        q->vz = voxl_fluid_rng_signed(&s->rng) * base * 0.5f;
        q->vy = base * (0.6f + 0.4f * voxl_fluid_rng_f(&s->rng));
        q->max_life = 0.4f + 0.6f * voxl_fluid_rng_f(&s->rng);
        q->life = q->max_life;
        spawned++;
    }
    return spawned;
}

int voxl_fluid_splash_update(voxl_fluid_splash_system *s, float gravity, float dt) {
    if (!s) return 0;
    int alive = 0;
    int top = 0;
    for (int i = 0; i < s->count; i++) {
        voxl_fluid_particle *q = &s->p[i];
        if (q->life <= 0.0f) continue;
        q->life -= dt;
        if (q->life <= 0.0f) { q->life = 0.0f; continue; }
        q->vy -= gravity * dt;
        q->x += q->vx * dt;
        q->y += q->vy * dt;
        q->z += q->vz * dt;
        alive++;
        top = i + 1;
    }
    s->count = top;   // shrink the scan range as tail slots die off
    return alive;
}

int voxl_fluid_splash_alive(const voxl_fluid_splash_system *s) {
    if (!s) return 0;
int alive = 0;
for (int i = 0;
i < s->count;
i++)
        if (s->p[i].life > 0.0f) alive++;
return alive;
}
