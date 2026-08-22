#ifndef SOUND_AUDIO3D_CONFIG_H
#define SOUND_AUDIO3D_CONFIG_H

// tunables for the 3d mixer. the old sound.c just logged events; this is the
// real thing that takes those events and turns them into stereo pcm.
//.h.

// output format. we mix in float internally and clamp to s16 on the way out.
#define AUDIO3D_SAMPLE_RATE     44100
#define AUDIO3D_CHANNELS        2          // stereo. mono sources, stereo bus.

// how many concurrent voices the mixer will service. anything past this gets
// voice-stolen (quietest one dies). 32 felt like plenty for a voxel game.
#define AUDIO3D_MAX_VOICES      32

// how many distinct clips a bank can hold. ids index straight into a table.
#define AUDIO3D_MAX_CLIPS       128

// distance model. beyond max_dist a source is fully silent and gets culled
// from the mix entirely (saves the resample work).
#define AUDIO3D_REF_DIST        2.0f       // below this, no attenuation
#define AUDIO3D_MAX_DIST        48.0f      // past this, silence
#define AUDIO3D_ROLLOFF         1.0f       // rolloff factor for inverse model

// speed of sound in blocks/sec. used for the doppler shift. air is ~343 m/s
// and a block is roughly a meter so this is close enough.
#define AUDIO3D_SPEED_OF_SOUND  343.0f
#define AUDIO3D_DOPPLER_SCALE   1.0f

// how wide the stereo image is. 1.0 = full hard pan at the sides.
#define AUDIO3D_PAN_WIDTH       0.85f

// occlusion lowpass. when a source is muffled (behind blocks) we drop the
// cutoff toward this. fully open is the nyquist.
#define AUDIO3D_OCCLUSION_CUTOFF_HZ  900.0f

// master gain headroom. we never let a single voice exceed this so a pile of
// them summing doesnt clip instantly. soft-knee limiter mops up the rest.
#define AUDIO3D_VOICE_HEADROOM  0.7f

// a tiny epsilon. floats, you know how it is.
#define AUDIO3D_EPS             1e-6f

#endif
