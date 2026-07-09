#include "gbuffer_debug.h"
#include "gbuffer_normal.h"

void gbuffer_debug_blit(gbuffer *g, gbuffer_view_mode mode) {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    glUseProgram(g->prog_debug);
    gbuffer_target_bind_textures(&g->target, 0);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, g->accum_tex);

    gl_set_uniform_int(g->prog_debug, "u_albedo",   0);
    gl_set_uniform_int(g->prog_debug, "u_normal",   1);
    gl_set_uniform_int(g->prog_debug, "u_material", 2);
    gl_set_uniform_int(g->prog_debug, "u_depth",    3);
    gl_set_uniform_int(g->prog_debug, "u_light",    4);
    // the shader switches on this. matches gbuffer_view_mode minus FINAL=0.
    gl_set_uniform_int(g->prog_debug, "u_mode", (int)mode);

    glBindVertexArray(g->fs_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}

const char *gbuffer_debug_mode_name(gbuffer_view_mode mode) {
    switch (mode) {
        case GBUFFER_VIEW_FINAL:    return "final";
        case GBUFFER_VIEW_ALBEDO:   return "albedo";
        case GBUFFER_VIEW_NORMAL:   return "normal";
        case GBUFFER_VIEW_MATERIAL: return "material";
        case GBUFFER_VIEW_DEPTH:    return "depth";
        case GBUFFER_VIEW_LIGHT:    return "light";
        default:                    return "?";
    }
}

gbuffer_material gbuffer_debug_probe_material(gbuffer *g, int px, int py) {
    // read the single material texel under (px,py). the g-buffer matches the
    // backbuffer size so screen coords map 1:1, but gl reads bottom-up.
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px >= g->w) px = g->w - 1;
    if (py >= g->h) py = g->h - 1;
    int flipped_y = g->h - 1 - py;

    unsigned char rgba[4] = { 0, 0, 0, 0 };
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g->target.fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + 2);  // material attachment
    glReadPixels(px, flipped_y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    gbuffer_mat_texel t = { rgba[0], rgba[1], rgba[2], rgba[3] };
    return gbuffer_material_unpack(t);
}
