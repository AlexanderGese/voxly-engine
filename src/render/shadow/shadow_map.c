#include "shadow_map.h"
#include "../../util/log.h"

#include <stddef.h>

int shadow_map_create(shadow_map *sm, int size) {
    sm->size   = size;
    sm->layers = SHADOW_CASCADE_COUNT;

    glGenTextures(1, &sm->depth_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sm->depth_array);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 size, size, sm->layers, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // border = 1.0 depth so anything sampled outside the map reads "lit", not
    // "in shadow". white border bug is a rite of passage, skip it.
    float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);

    // hardware pcf: sampler compares ref vs stored depth and returns 0..1.
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glGenFramebuffers(1, &sm->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, sm->fbo);
    // attach layer 0 just to validate completeness; we re-attach per pass.
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              sm->depth_array, 0, 0);
    glDrawBuffer(GL_NONE);   // depth only, no color
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("shadow_map: fbo incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shadow_map_destroy(sm);
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOGI("shadow_map: %d layers @ %dx%d depth32f", sm->layers, size, size);
    return 1;
}

void shadow_map_destroy(shadow_map *sm) {
    if (sm->fbo)         glDeleteFramebuffers(1, &sm->fbo);
    if (sm->depth_array) glDeleteTextures(1, &sm->depth_array);
    sm->fbo = sm->depth_array = 0;
}

void shadow_map_begin_layer(shadow_map *sm, int layer) {
    glBindFramebuffer(GL_FRAMEBUFFER, sm->fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              sm->depth_array, 0, layer);
    glViewport(0, 0, sm->size, sm->size);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void shadow_map_end(const shadow_map *sm) {
    (void)sm;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void shadow_map_bind_sampler(const shadow_map *sm, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sm->depth_array);
}
