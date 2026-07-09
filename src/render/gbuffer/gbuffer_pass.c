#include "gbuffer_pass.h"
#include "gbuffer_geometry.h"
#include "gbuffer_accum.h"
#include "gbuffer_shade.h"
#include "../../util/log.h"

void gbuffer_pass_run(gbuffer *g, const gbuffer_frame *frame) {
    if (!frame || !frame->draw) {
        LOGE("gbuffer_pass_run: no draw callback");
        return;
    }

    // 1. geometry: fill albedo/normal/material/depth
    gbuffer_geometry_begin(g, frame->atlas);
    frame->draw(g, frame->user);
    gbuffer_geometry_end(g);

    // 2. light accumulation into the hdr accum buffer
    if (frame->lights) {
        gbuffer_accumulate(g, frame->lights, frame->frustum, frame->eye);
    } else {
        // nothing to accumulate, just clear the buffer so shade reads zero
        glBindFramebuffer(GL_FRAMEBUFFER, g->accum_fbo);
        glViewport(0, 0, g->w, g->h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        g->culled.count = 0;
    }

    // 3. final combine to the currently-bound framebuffer
    glViewport(0, 0, g->w, g->h);
    gbuffer_shade(g);
}
