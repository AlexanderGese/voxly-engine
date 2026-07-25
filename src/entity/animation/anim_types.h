#ifndef ENTITY_ANIMATION_ANIM_TYPES_H
#define ENTITY_ANIMATION_ANIM_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../../math/mat4.h"
// shared types for the skeletal animation subsystem.
//
// this is the "new" animation path — the old entity/animation.{c,h} is the
// procedural limb-wiggler for mobs. that one just spits out a few sine angles.
// this one is the real deal: a bone hierarchy, sampled keyframe clips, and a
// little blend graph so we can cross-fade walk->run without it looking like the
// poor thing teleported. kept render-free on purpose; we produce skinning
// matrices and the render layer can do whatever it wants with them.
//
// rotations are quaternions because euler bones gimbal-lock the moment you look
// at them funny. the engine had no quat type so there's one here, prefixed.
#define ANIM_MAX_BONES        64   // skinning palette is 64 mat4s, plenty for a mob
#define ANIM_MAX_NAME         32
#define ANIM_BONE_NONE        (-1) // parent index for roots
// unit quaternion. w is the scalar part. we keep them normalized-ish and
// re-normalize after blends since nlerp drifts.
typedef struct {
    float x, y, z, w;
} animation_quat;
// a TRS (translate / rotate / scale) transform. one per bone, in parent space.
// stored decomposed so we can lerp each channel independently — lerping a
// composed matrix gives you shearing garbage.
typedef struct {
    vec3           translation;
    animation_quat rotation;
    vec3           scale;
} animation_transform;
// which channel a keyframe track drives. a clip has up to three tracks per bone.
typedef enum {
    ANIM_CHANNEL_TRANSLATION = 0,
    ANIM_CHANNEL_ROTATION,
    ANIM_CHANNEL_SCALE,
    ANIM_CHANNEL_COUNT
} animation_channel_kind;
// how a sampler steps between keyframes. step is for robotic/snappy stuff,
// linear is the default, and we treat cubic as linear for now (no tangents
// authored yet — see anim_sampler.c, there's a grumpy note about it).
typedef enum {
    ANIM_INTERP_STEP = 0,
    ANIM_INTERP_LINEAR,
    ANIM_INTERP_CUBIC
} animation_interp;
// one bone in the rest skeleton. parent comes before child in the array
// (topological order) so a single forward pass resolves model matrices.
typedef struct {
    char                name[ANIM_MAX_NAME];
    int                 parent;        // index into skeleton bones, or ANIM_BONE_NONE
    animation_transform local;         // bind/rest pose, parent space
    mat4                inverse_bind;   // model-space bind inverse, for skinning
} animation_bone;
// the rest skeleton. immutable once built; poses reference it by bone count.
typedef struct {
    animation_bone bones[ANIM_MAX_BONES];
    int            bone_count;
} animation_skeleton;
// a single keyframe value on a track. only one of the unions is meaningful,
// picked by the owning track's channel kind. time is in seconds from clip start.
typedef struct {
    float time;
    union {
        vec3           vec;   // translation / scale
        animation_quat quat;  // rotation
    } v;
} animation_keyframe;
// a keyframe track: all samples for one (bone, channel) pair. keys are sorted
// by time. owns its key array (plain malloc, freed by clip_free).
typedef struct {
    int                 bone;
    animation_channel_kind kind;
    animation_interp    interp;
    animation_keyframe *keys;     // sorted ascending by .time
    int                 key_count;
} animation_track;
// a named clip: a bundle of tracks plus a duration. looping is a playback
// decision (sampler/controller), not baked in here.
typedef struct {
    char             name[ANIM_MAX_NAME];
    animation_track *tracks;
    int              track_count;
    float            duration;    // seconds; cached max key time
} animation_clip;
// a pose is the evaluated local transforms for every bone at some instant.
// this is the currency the whole subsystem trades in: samplers fill them,
// blenders mix them, the skeleton turns them into matrices.
typedef struct {
    animation_transform locals[ANIM_MAX_BONES];
    int                 bone_count;
} animation_pose;
#endif
