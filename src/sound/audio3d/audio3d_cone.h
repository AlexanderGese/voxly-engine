#ifndef SOUND_AUDIO3D_CONE_H
#define SOUND_AUDIO3D_CONE_H

#include "audio3d_types.h"

// directional sources. think a record player, a furnace roar, anything that
// shouts in one direction. inside the inner cone you get full gain, outside the
// outer cone you get a reduced gain, and in between it lerps. this mirrors the
// openal cone model but stays in our own structs.

// set a voice's cone from human-friendly angles (degrees, full-cone angle).
// pass inner_deg >= 360 to make the source omnidirectional again.
void  audio3d_cone_set(audio3d_voice *v, vec3 dir,
                       float inner_deg, float outer_deg, float outer_gain);

// clear the cone -> omnidirectional.
void  audio3d_cone_clear(audio3d_voice *v);

// gain multiplier 0..1 for a source at src_pos facing per its cone, heard from
// listener_pos. returns 1.0 for omnidirectional sources. listener_pos is the
// point we're listening from; the cone is evaluated along source->listener.
float audio3d_cone_gain(const audio3d_voice *v, vec3 listener_pos);

#endif
