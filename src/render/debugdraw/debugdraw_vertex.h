#ifndef RENDER_DEBUGDRAW_VERTEX_H
#define RENDER_DEBUGDRAW_VERTEX_H

#include <stddef.h>   // offsetof
#include "../../math/vec3.h"
#include "debugdraw_color.h"

// the one vertex format the whole subsystem uses. position + packed color.
// 16 bytes, aligns nicely. lines and points share it; the point shader
// just reads gl_PointSize from a uniform instead of per-vertex.

typedef struct {
    float   x, y, z;
    ddcolor color;     // packed rgba, uploaded as 4 normalized ubytes
} ddvert;

static inline ddvert ddvert_make(vec3 p, ddcolor c) {
    ddvert v;
    v.x = p.x;
    v.y = p.y;
    v.z = p.z;
    v.color = c;
    return v;
}

// attribute layout shared by debugdraw_gl.c and the .vert shader:
// location 0 -> vec3  position   (offset 0)
// location 1 -> vec4  color      (offset 12, GL_UNSIGNED_BYTE, normalized)
#define DDVERT_STRIDE        (int)sizeof(ddvert)
#define DDVERT_POS_OFFSET    0
#define DDVERT_COL_OFFSET    ((int)offsetof(ddvert, color))

#endif
