#ifndef RENDER_GBUFFER_SHADE_H
#define RENDER_GBUFFER_SHADE_H

#include "gbuffer.h"

// final combine pass. samples the g-buffer albedo + the accumulation buffer,
// adds the flat ambient term and the material's emissive, tonemaps, and
// blits the result to whatever framebuffer is currently bound (usually 0,
// the default backbuffer, or the post-process chain's input fbo).
//
// if a debug view mode is active this defers to gbuffer_debug instead of
// running the real combine.

// exposure used by the reinhard tonemap in the shade shader. higher = brighter.
#define GBUFFER_DEFAULT_EXPOSURE 1.1f

void gbuffer_shade(gbuffer *g);

// just the tonemap + ambient combine, no debug branch. exposed so the post
// chain can call it directly when it owns the final blit.
void gbuffer_shade_combine(gbuffer *g, float exposure);

#endif
