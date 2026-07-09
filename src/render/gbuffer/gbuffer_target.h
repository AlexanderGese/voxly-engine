#ifndef RENDER_GBUFFER_TARGET_H
#define RENDER_GBUFFER_TARGET_H
#include "../gl.h"
// the raw multi-render-target framebuffer the geometry pass writes into.
// three color attachments + a sampled depth texture. nothing fancy, just
// the storage. the actual semantics of each channel live in gbuffer.h.
//
// attachment 0 (RGBA8)   albedo.rgb + ao in .a
// attachment 1 (RGB10A2) world normal packed into .rgb, .a unused-ish
// attachment 2 (RGBA8)   material: roughness, metallic, emissive, flags
#define GBUFFER_MRT_COUNT 3
typedef struct {
    glid fbo;
    glid tex[GBUFFER_MRT_COUNT];
    glid depth_tex;
    int  w, h;
} gbuffer_target;
int  gbuffer_target_create(gbuffer_target *t, int w, int h);
void gbuffer_target_destroy(gbuffer_target *t);
void gbuffer_target_bind(const gbuffer_target *t);
void gbuffer_target_bind_textures(const gbuffer_target *t, int base);
void gbuffer_target_resize(gbuffer_target *t, int w, int h);
#endif
