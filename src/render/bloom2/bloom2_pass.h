#ifndef RENDER_BLOOM2_PASS_H
#define RENDER_BLOOM2_PASS_H

#include "../gl.h"
#include "bloom2_chain.h"
#include "bloom2_programs.h"
#include "bloom2_params.h"
#include "bloom2_quad.h"
#include "bloom2_tint.h"

// the individual stages of the bloom chain. these are the meat: they wire up
// textures, set uniforms and draw the fullscreen tri into the right target.
// the manager just calls them in order.

// bright pass: read the scene color, knee-threshold it, clamp fireflies, and
// write the result into mip[0]. this is the only pass that touches the
// scene texture.
void bloom2_pass_bright(const bloom2_programs *prog,
                        const bloom2_chain *chain,
                        const bloom2_params *params,
                        const bloom2_quad *quad,
                        glid scene_tex);

// downsample: walk mip[0]->mip[count-1], each level a 13-tap filtered
// reduction of the level above. progressively blurs and shrinks.
void bloom2_pass_downsample(const bloom2_programs *prog,
                            const bloom2_chain *chain,
                            const bloom2_quad *quad);

// upsample: walk back up mip[count-1]->mip[0], additively blending each
// (tent-filtered) lower mip into the one above. builds the wide soft glow.
void bloom2_pass_upsample(const bloom2_programs *prog,
                          const bloom2_chain *chain,
                          const bloom2_params *params,
                          const bloom2_tint *tint,
                          const bloom2_quad *quad);

// composite: additively blend mip[0] over whatever framebuffer is bound
// (usually the backbuffer). dst_w/dst_h set the viewport back to full res.
void bloom2_pass_composite(const bloom2_programs *prog,
                           const bloom2_chain *chain,
                           const bloom2_params *params,
                           const bloom2_quad *quad,
                           int dst_w, int dst_h);

#endif
