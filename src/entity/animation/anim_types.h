#ifndef ENTITY_ANIMATION_ANIM_TYPES_H
#define ENTITY_ANIMATION_ANIM_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#define ANIM_MAX_BONES        64   // skinning palette is 64 mat4s, plenty for a mob
#define ANIM_MAX_NAME         32
#define ANIM_BONE_NONE        (-1) // parent index for roots
typedef struct {
    float x, y, z, w;
} animation_quat;
typedef struct {
    vec3           translation;
    animation_quat rotation;
    vec3           scale;
} animation_transform;
#endif
