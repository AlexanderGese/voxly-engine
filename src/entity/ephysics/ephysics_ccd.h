#ifndef ENTITY_EPHYSICS_CCD_H
#define ENTITY_EPHYSICS_CCD_H
#include "ephysics_types.h"
#include "ephysics_broadphase.h"
#include "ephysics_contact.h"
// continuous-ish collision: keep fast bodies from tunnelling through thin walls.
// the swept-aabb resolver is already continuous within one sweep, but the
// broadphase candidate set is gathered once around the *start* box plus a skin.
// a body moving faster than ~a block per tick can leave that region before the
// sweep finishes, so we split the move into sub-steps short enough that each one
// stays inside a freshly gathered candidate set.
//
// this is purely a driver around ephysics_gather + ephysics_resolve; no new
// collision math. it also feeds the contact manifold so callers downstream still
// see every hit even though the move was chopped up.
// largest move length (meters) we allow per sub-step before splitting. a bit
// under one block so the one-block broadphase skin always covers it.
#define EPHYS_CCD_MAX_STEP 0.45f
// hard cap on sub-steps for one move, so a teleport-sized delta cant lock the
// frame. anything past this just gets clamped and we eat the tunnelling.
#define EPHYS_CCD_MAX_SUBSTEPS 12
int ephysics_ccd_substeps(vec3 delta);
vec3 ephysics_ccd_resolve(world *w, ephys_body *b, vec3 delta, int iters,
                          ephys_manifold *man);
#endif
