#ifndef ENTITY_EPHYSICS_CAMERA_H
#define ENTITY_EPHYSICS_CAMERA_H

#include "ephysics_types.h"
#include "../../world/world.h"

// camera collision. not entity motion exactly, but it's the same swept-box
// problem so it lives in the physics module instead of render. two jobs:
// - third-person: pull the camera in toward the eye so it never sits inside a
// wall behind the player.
// - first-person: a tiny near-clip probe so you cant shove the eye through a
// block by standing in a corner.
//
// the math is the voxel raycast from ephysics_query plus a small sphere skin so
// the near plane doesnt poke through.

typedef struct {
    vec3  eye;         // where the camera wants to be (player eye)
    vec3  dir;         // look direction, unit. camera sits behind the eye on -dir
    float distance;    // desired boom length for third person (0 = first person)
    float radius;      // camera collision sphere radius, keeps the near plane out
} ephys_camera_probe;

// resolve the boom against the world. returns the final camera position: eye
// minus dir*clamped_distance, where clamped_distance is shortened to the first
// wall the boom would clip, minus the radius skin. for a first-person probe
// (distance 0) it just nudges the eye out of any block it's buried in.
vec3 ephysics_camera_resolve(world *w, const ephys_camera_probe *p);

// how far back the camera can sit before clipping, in meters. exposed so the
// renderer can ease the boom toward this instead of snapping (snapping looks
// awful when you walk past a doorway).
float ephysics_camera_clear_dist(world *w, const ephys_camera_probe *p);

#endif
