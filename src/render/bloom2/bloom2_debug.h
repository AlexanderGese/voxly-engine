#ifndef RENDER_BLOOM2_DEBUG_H
#define RENDER_BLOOM2_DEBUG_H

#include <stddef.h>

#include "bloom2.h"

// dumps a one-liner per mip describing the pyramid + current params. wired to
// the F3-style debug overlay, but also useful from the console. nothing here
// touches gl state, it just reads the structs.

// estimate the vram the chain occupies, in bytes. rgba16f = 8 bytes/texel.
size_t bloom2_debug_chain_bytes(const bloom2 *b);

// write a human readable summary into buf (truncated to cap). returns the
// number of chars it would have written (snprintf semantics).
int    bloom2_debug_summary(const bloom2 *b, char *buf, size_t cap);

// log the whole thing at info level. handy after a resize.
void   bloom2_debug_log(const bloom2 *b);

#endif
