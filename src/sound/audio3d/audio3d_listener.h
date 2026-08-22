#ifndef SOUND_AUDIO3D_LISTENER_H
#define SOUND_AUDIO3D_LISTENER_H

#include "audio3d_types.h"

// the listener. set it from the camera each frame and the mixer reads it.
// keeping the basis orthonormal is the whole point — pan and doppler both
// project source vectors onto right/fwd, so a skewed basis sounds wrong.

void audio3d_listener_init(audio3d_listener *l);

// build the basis from a look direction and a world up. handles the degenerate
// case where you look straight up/down (fwd parallel to up).
void audio3d_listener_set_orientation(audio3d_listener *l, vec3 fwd, vec3 up);

// convenience: yaw/pitch in radians, same convention the camera uses.
void audio3d_listener_set_angles(audio3d_listener *l, float yaw, float pitch);

void audio3d_listener_set_pos(audio3d_listener *l, vec3 pos);

// feed a new position and derive velocity from the delta. dt in seconds.
// pass dt<=0 to teleport (zero velocity, no doppler pop).
void audio3d_listener_move(audio3d_listener *l, vec3 pos, float dt);

void audio3d_listener_set_gain(audio3d_listener *l, float gain);

// project a world point into listener-local space (right/up/fwd axes).
vec3 audio3d_listener_local(const audio3d_listener *l, vec3 world);

#endif
