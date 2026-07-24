#ifndef ENTITY_ANIMATION_ANIM_CLIP_H
#define ENTITY_ANIMATION_ANIM_CLIP_H

#include "anim_types.h"

// keyframe clip construction and channel sampling. a clip owns its tracks and
// each track owns its key array (plain malloc). build with init + add_track +
// push_key, then finalize to cache the duration. free with clip_free.

void animation_clip_init(animation_clip *clip, const char *name);

// add an empty track for (bone, channel). returns the track index, or -1 if the
// track array couldn't grow. interp defaults to linear; override after.
int  animation_clip_add_track(animation_clip *clip, int bone,
                              animation_channel_kind kind);

// push a vec key (translation/scale). keys MUST be pushed in ascending time;
// we assert order rather than sort, since exporters already emit sorted.
void animation_clip_push_vec(animation_clip *clip, int track, float time, vec3 v);

// push a rotation key. same ascending-time rule.
void animation_clip_push_quat(animation_clip *clip, int track, float time,
                              animation_quat q);

// set how a track steps between its keys.
void animation_clip_set_interp(animation_clip *clip, int track,
                               animation_interp interp);

// cache duration = max key time across all tracks. call once after pushing.
void animation_clip_finalize(animation_clip *clip);

// sample one track at `time` (already wrapped/clamped by the caller). writes the
// interpolated value into out_vec or out_quat depending on the track's channel.
// returns 1 if it wrote, 0 if the track was empty.
int  animation_clip_sample_track(const animation_track *track, float time,
                                 vec3 *out_vec, animation_quat *out_quat);

void animation_clip_free(animation_clip *clip);

#endif
