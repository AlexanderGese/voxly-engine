#ifndef SOUND_AUDIO3D_PAN_H
#define SOUND_AUDIO3D_PAN_H

#include "audio3d_types.h"
#include "audio3d_listener.h"

// stereo panner. takes a source position, the listener basis, and a base gain,
// and spits out left/right channel gains. constant-power law so a source
// sweeping past you keeps a steady loudness instead of dipping in the middle.

typedef struct {
    float left;
    float right;
} audio3d_pan;

// pan from a fully resolved azimuth (-1 hard left .. +1 hard right). split out
// so the mixer can reuse it for non-positional ambience too.
audio3d_pan audio3d_pan_from_azimuth(float azimuth, float gain, float width);

// the real entry point: resolve azimuth from world geometry, then pan.
// sources directly behind get a slight gain dip to fake the head shadow.
audio3d_pan audio3d_pan_spatial(const audio3d_listener *l, vec3 src_pos,
                                float gain, float width);

#endif
