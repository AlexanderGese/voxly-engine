#ifndef RENDER_SHADERMAN_TYPES_H
#define RENDER_SHADERMAN_TYPES_H

// shared types for the shader manager. kept in its own header so the compile
// and reload bits dont have to drag in the whole world. handles are opaque
// indices into the manager's program table, NOT raw gl program ids — that way
// a hot-reload can swap the underlying gl program out from under you without
// invalidating anything anyone is holding.

#include "../gl.h"          // glid
#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "../../math/mat4.h"

#include <stdint.h>
#include <stdbool.h>

// max we bother supporting. raise if you write a megashader, but you wont.
#define SHADERMAN_MAX_PROGRAMS     64
#define SHADERMAN_MAX_UNIFORMS     48   // per program
#define SHADERMAN_MAX_STAGES        3   // vert + frag + optional geom
#define SHADERMAN_MAX_INCLUDES     16   // include depth/count guard
#define SHADERMAN_NAME_LEN         48
#define SHADERMAN_PATH_LEN        192

// a handle of 0 is the invalid/null program. real handles are 1-based so a
// zeroed struct reads as "no shader".
typedef uint16_t shader_handle;
#define SHADER_HANDLE_NONE  ((shader_handle)0)

typedef enum {
    SHADER_STAGE_VERT = 0,
    SHADER_STAGE_FRAG = 1,
    SHADER_STAGE_GEOM = 2,
} shader_stage_kind;

// what kind of value lives in a cached uniform slot. we keep the last value
// pushed so we can skip redundant gl calls (those add up in a chunk-heavy
// frame). u_dirty marks "location known, value not yet uploaded".
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

// one compilation stage: a path on disk and the mtime we last saw it at.
typedef struct {
    char     path[SHADERMAN_PATH_LEN];
    int64_t  mtime;            // seconds, 0 = unknown / not stat-able
    bool     present;          // path actually set for this stage
} shader_stage;

// a managed program. owns its gl id plus a small uniform cache.
typedef struct {
    char          name[SHADERMAN_NAME_LEN];
    glid          prog;        // current live gl program, 0 if failed
    shader_stage  stages[SHADERMAN_MAX_STAGES];

    shader_uniform_slot uniforms[SHADERMAN_MAX_UNIFORMS];
    int           uniform_count;

    uint32_t      reload_gen;  // bumped every successful (re)link
    bool          in_use;      // slot occupied in the manager table
    bool          ok;          // last compile/link succeeded
} shader_program;

#endif
