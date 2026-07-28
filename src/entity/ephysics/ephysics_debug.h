#ifndef ENTITY_EPHYSICS_DEBUG_H
#define ENTITY_EPHYSICS_DEBUG_H

#include "ephysics_types.h"
#include "ephysics_broadphase.h"

// debug introspection. the render layer's wireframe pass can draw the candidate
// boxes and the body box; this just hands them over as plain aabbs plus a tiny
// status string for the f3 overlay. no GL here, stays renderer-agnostic.

// human-readable flag string, e.g. "grnd water step". writes into `buf` (must
// be >= 64 bytes) and returns it.
const char *ephysics_flags_str(uint32_t flags, char *buf, int cap);

// copy up to `cap` candidate boxes into `out`, returns how many were written.
// handy for the wireframe debug draw of what the body is colliding against.
int ephysics_debug_boxes(const ephys_candidates *c, aabb *out, int cap);

// one-line summary of a body's state for logging/overlay. writes into `buf`.
void ephysics_debug_line(const ephys_body *b, char *buf, int cap);

#endif
