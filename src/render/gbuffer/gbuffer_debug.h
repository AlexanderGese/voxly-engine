#ifndef RENDER_GBUFFER_DEBUG_H
#define RENDER_GBUFFER_DEBUG_H

#include "gbuffer.h"
#include "gbuffer_material.h"

// debug channel visualiser. blits a single g-buffer channel to the screen so
// you can eyeball what the geometry pass actually wrote. wired to a key in
// the renderer (cycles through gbuffer_view_mode). also has a tiny cpu-side
// readback for the pixel under the crosshair, handy when a material looks
// wrong and you want the exact packed bytes.

// blit the requested channel fullscreen via prog_debug. `mode` is one of the
// non-FINAL gbuffer_view_mode values.
void gbuffer_debug_blit(gbuffer *g, gbuffer_view_mode mode);

// human-readable name for a view mode, for the hud label.
const char *gbuffer_debug_mode_name(gbuffer_view_mode mode);

// read back the four material bytes at screen pixel (px,py). y is flipped to
// match gl's bottom-left origin internally. returns the unpacked material.
gbuffer_material gbuffer_debug_probe_material(gbuffer *g, int px, int py);

#endif
