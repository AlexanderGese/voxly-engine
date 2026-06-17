#ifndef WORLD_FLUID_SPLASH_H
#define WORLD_FLUID_SPLASH_H

#include <stdint.h>
#include <stdbool.h>

// splash particles spawned when something hits a fluid surface. dead-simple
// ballistic particles with a lifetime. self-contained, no render calls; the
// game reads positions out and draws them however it likes.

typedef struct {
    float x, y, z;       // position
    float vx, vy, vz;    // velocity
    float life;          // seconds remaining, <=0 is dead
    float max_life;
} voxl_fluid_particle;

#define VOXL_FLUID_MAX_SPLASH 128

typedef struct voxl_fluid_splash_system {
    voxl_fluid_particle p[VOXL_FLUID_MAX_SPLASH];
    int    count;        // high-water mark of live slots
    uint64_t rng;        // local xorshift state
} voxl_fluid_splash_system;

void voxl_fluid_splash_init(voxl_fluid_splash_system *s, uint64_t seed);

// spawn a burst of n droplets at a point with a given impact speed. returns
// how many were actually spawned (capped by free slots).
int  voxl_fluid_splash_spawn(voxl_fluid_splash_system *s,
                             float x, float y, float z,
                             float impact_speed, int n);

// advance all particles by dt under gravity. returns live particle count.
int  voxl_fluid_splash_update(voxl_fluid_splash_system *s, float gravity, float dt);

// number of currently-alive particles.
int  voxl_fluid_splash_alive(const voxl_fluid_splash_system *s);

#endif
