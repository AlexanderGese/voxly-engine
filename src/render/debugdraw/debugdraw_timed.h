#ifndef RENDER_DEBUGDRAW_TIMED_H
#define RENDER_DEBUGDRAW_TIMED_H

#include "../../math/vec3.h"
#include "debugdraw_color.h"

// timed entries: lines/boxes that stick around for a few seconds and fade
// out. great for "this raycast happened" or "ai picked this path" markers
// you want to actually see instead of a one-frame flicker.
//
// these are stored cpu-side and re-emitted into the line buckets every
// flush until their ttl runs out. the color alpha is scaled by remaining
// life so they dim toward the end.

typedef struct debugdraw debugdraw;  // fwd

typedef enum {
    DD_TIMED_LINE = 0,
    DD_TIMED_BOX,
    DD_TIMED_CROSS
} ddtimed_kind;

typedef struct ddtimed {
    ddtimed_kind kind;
    vec3    a;          // line: endpoint a / box: min  / cross: center
    vec3    b;          // line: endpoint b / box: max  / cross: (size in x)
    ddcolor color;
    float   spawn;      // dd->now when added
    float   ttl;        // total lifetime, seconds
    int     depth;      // depth-tested?
} ddtimed;

// add timed primitives. ttl in seconds. they decay/fade automatically.
void debugdraw_timed_line(debugdraw *dd, vec3 a, vec3 b, ddcolor c, float ttl);
void debugdraw_timed_box(debugdraw *dd, vec3 mn, vec3 mx, ddcolor c, float ttl);
void debugdraw_timed_cross(debugdraw *dd, vec3 center, float size,
                           ddcolor c, float ttl);

// re-emit all live timed entries into the frame buckets, dropping any that
// have expired. called by debugdraw_flush before upload. returns the number
// still alive.
int  debugdraw_timed_tick(debugdraw *dd);

// drop everything immediately
void debugdraw_timed_clear(debugdraw *dd);

#endif
