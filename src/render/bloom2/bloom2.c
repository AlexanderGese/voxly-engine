#include "bloom2.h"
#include "bloom2_pass.h"
#include "../../util/log.h"

#include <stddef.h>

int bloom2_init(bloom2 *b, int w, int h) {
    b->ready  = 0;
    b->frames = 0;

    bloom2_params_default(&b->params);
    bloom2_params_sanitize(&b->params);
    bloom2_tint_default(&b->tint);

    bloom2_quad_create(&b->quad);

    if (!bloom2_programs_load(&b->prog)) {
        // shaders missing — keep the struct valid so run() can no-op.
        b->chain.count = 0;
        return 0;
    }

    if (!bloom2_chain_build(&b->chain, w, h, b->params.mip_count)) {
        LOGW("bloom2: chain build failed, disabling");
        bloom2_programs_destroy(&b->prog);
        return 0;
    }

    // the chain may have fewer mips than we asked for on a small window —
    // sync the param so the rest of the code agrees.
    b->params.mip_count = b->chain.count;
    b->ready = 1;
    return 1;
}

void bloom2_destroy(bloom2 *b) {
    bloom2_chain_destroy(&b->chain);
    bloom2_programs_destroy(&b->prog);
    bloom2_quad_destroy(&b->quad);
    b->ready = 0;
}

void bloom2_resize(bloom2 *b, int w, int h) {
    if (!bloom2_programs_ok(&b->prog)) return;   // never got off the ground
    if (b->chain.base_w == w && b->chain.base_h == h) return;

    bloom2_chain_destroy(&b->chain);
    if (!bloom2_chain_build(&b->chain, w, h, BLOOM2_MAX_MIPS)) {
        b->ready = 0;
        return;
    }
    b->params.mip_count = b->chain.count;
    b->ready = 1;
}

void bloom2_run(bloom2 *b, glid scene_tex, glid dst, int dst_w, int dst_h) {
    if (!b->ready || !b->params.enabled) return;
    if (b->chain.count < 1) return;

    bloom2_params_sanitize(&b->params);

    // save the bits of gl state we stomp so the caller doesnt have to. the
    // renderer is picky about depth test leaking between subsystems.
    GLboolean had_depth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean had_blend = glIsEnabled(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // 1. bright pass into mip[0]
    bloom2_pass_bright(&b->prog, &b->chain, &b->params, &b->quad, scene_tex);

    // 2. downsample pyramid (only meaningful with >1 mip)
    if (b->chain.count > 1)
        bloom2_pass_downsample(&b->prog, &b->chain, &b->quad);

    // 3. upsample + additive blend back up the pyramid
    if (b->chain.count > 1)
        bloom2_pass_upsample(&b->prog, &b->chain, &b->params, &b->tint, &b->quad);

    // 4. composite mip[0] over the destination framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, dst);
    bloom2_pass_composite(&b->prog, &b->chain, &b->params, &b->quad, dst_w, dst_h);

    // restore
    if (had_depth) glEnable(GL_DEPTH_TEST);
    if (!had_blend) glDisable(GL_BLEND);
    glBindVertexArray(0);

    b->frames++;
}

bloom2_params *bloom2_get_params(bloom2 *b) {
    return &b->params;
}
