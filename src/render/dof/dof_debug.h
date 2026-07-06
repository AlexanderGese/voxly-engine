#ifndef RENDER_DOF_DEBUG_H
#define RENDER_DOF_DEBUG_H

#include "dof.h"

#include <stddef.h>

// debug introspection for the dof chain. mirrors bloom2_debug: pure reads of
// the live structs plus a couple of formatting helpers wired to the F3-style
// overlay. nothing here touches gl state, so it's safe to call from anywhere.

// estimate the vram the four half-res buffers occupy, in bytes. each is
// rgba16f = 8 bytes/texel.
size_t dof_debug_buffer_bytes(const dof *d);

// the names of the focus states, for the readout. index with dof_focus_state.
const char *dof_debug_state_name(int state);

// the name of a debug view mode (normal/coc/near/far). index with the
// DOF_VIEW_* enum; returns "?" out of range.
const char *dof_debug_view_name(int view);

// write a multi-field human readable summary into buf (truncated to cap).
// returns snprintf-style would-have-written length. covers focus distance,
// state, aperture, tap count and buffer size.
int    dof_debug_summary(const dof *d, char *buf, size_t cap);

// log the whole thing at info level. handy after a resize or a param tweak.
void   dof_debug_log(const dof *d);

// sanity self-check run from the console: builds a kernel + lens, sweeps the
// coc, and asserts the invariants (coc zero at focus, signs correct, kernel
// uniform). returns the count of checks that PASSED; a perfect run returns the
// total. logs each failure.
int    dof_debug_selftest(void);

#endif
