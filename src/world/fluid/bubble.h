#ifndef WORLD_FLUID_BUBBLE_H
#define WORLD_FLUID_BUBBLE_H

#include "fluid_cell.h"
#include <stdint.h>

// rising air bubbles inside a fluid body. they drift up, wobble a bit, and
// pop at the surface. used for ambient underwater effect and to mark bubble
// columns (soul-sand style updrafts) later.

typedef struct {
    float x, y, z;
    float wobble_phase;
    float radius;
    float rise_speed;
    uint8_t alive;
} voxl_fluid_bubble;

#define VOXL_FLUID_MAX_BUBBLES 96

typedef struct voxl_fluid_bubble_system {
    voxl_fluid_bubble b[VOXL_FLUID_MAX_BUBBLES];
    int count;
    uint64_t rng;
    float emit_acc;     // accumulator for steady emission
} voxl_fluid_bubble_system;

void voxl_fluid_bubble_init(voxl_fluid_bubble_system *s, uint64_t seed);

// spawn a single bubble at a point. returns true if a slot was free.
bool voxl_fluid_bubble_spawn(voxl_fluid_bubble_system *s, float x, float y, float z);

// steadily emit bubbles from a point at `rate` per second while called each
// frame. returns number spawned this call.
int  voxl_fluid_bubble_emit(voxl_fluid_bubble_system *s,
                            float x, float y, float z, float rate, float dt);

// rise + wobble. bubbles above surface_y pop. returns live count.
int  voxl_fluid_bubble_update(voxl_fluid_bubble_system *s, float surface_y, float dt);

#endif
