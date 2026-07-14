#ifndef RENDER_SHADERMAN_TYPES_H
#define RENDER_SHADERMAN_TYPES_H
#include "../gl.h"          // glid
#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "../../math/mat4.h"
#include <stdint.h>
#include <stdbool.h>
#define SHADERMAN_MAX_PROGRAMS     64
#define SHADERMAN_MAX_UNIFORMS     48   // per program
#define SHADERMAN_MAX_STAGES        3   // vert + frag + optional geom
#define SHADERMAN_MAX_INCLUDES     16   // include depth/count guard
#define SHADERMAN_NAME_LEN         48
#define SHADERMAN_PATH_LEN        192
#define SHADER_HANDLE_NONE  ((shader_handle)0)
typedef enum {
    SHADER_STAGE_VERT = 0,
    SHADER_STAGE_FRAG = 1,
    SHADER_STAGE_GEOM = 2,
} shader_stage_kind;
typedef enum {
    SHADER_U_NONE = 0,
    SHADER_U_INT,
    SHADER_U_FLOAT,
    SHADER_U_VEC3,
    SHADER_U_VEC4,
    SHADER_U_MAT4,
} shader_uniform_kind;
typedef struct {
    char  name[SHADERMAN_NAME_LEN];
    int   location;            // glGetUniformLocation result, -1 if absent
    uint32_t name_hash;        // fnv1a of name, for fast cache probing
    shader_uniform_kind kind;  // last kind written (for redundancy checks)
    bool  dirty;               // value changed since last upload
    union {
        int   i;
        float f;
        vec3  v3;
        vec4  v4;
        float m16[16];
    } val;
} shader_uniform_slot;
#endif
