#include "bloom2_blur.h"
#include "../../util/log.h"

#include <stddef.h>

// push the packed gaussian into the bound program. both the h and v shaders
// declare the same uniform names so we can reuse this for either direction.
static void upload_taps(const bloom2_blur *bl, glid prog) {
    gl_set_uniform_int(prog, "u_taps", bl->packed_taps);
    for (int i = 0; i < bl->packed_taps; i++) {
        // glsl arrays need per-element names with this tiny uniform helper.
        char wn[24], on[24];
        // cheap itoa; index never exceeds RADIUS+1 so two digits is plenty.
        int d0 = i / 10, d1 = i % 10;
        int p = 0;
        const char *wp = "u_weight[";
        const char *op = "u_offset[";
        for (int k = 0; wp[k]; k++) { wn[p] = wp[k]; on[p] = op[k]; p++; }
        if (d0) { wn[p] = (char)('0'+d0); on[p] = (char)('0'+d0); p++; }
        wn[p] = (char)('0'+d1); on[p] = (char)('0'+d1); p++;
        wn[p] = ']'; on[p] = ']'; p++;
        wn[p] = 0;   on[p] = 0;
        gl_set_uniform_float(prog, wn, bl->packed_weight[i]);
        gl_set_uniform_float(prog, on, bl->packed_offset[i]);
    }
}

void bloom2_blur_set_sigma(bloom2_blur *bl, float sigma) {
    bloom2_gauss_build(&bl->gauss, sigma);
    bl->packed_taps = bloom2_gauss_pack_linear(&bl->gauss,
                                               bl->packed_weight,
                                               bl->packed_offset);
}

int bloom2_blur_init(bloom2_blur *bl, int w, int h, float sigma) {
    bl->ready  = 0;
    bl->prog_h = 0;
    bl->prog_v = 0;

    bloom2_blur_set_sigma(bl, sigma);

    if (!bloom2_target_create(&bl->scratch, w, h)) {
        LOGW("bloom2: blur scratch alloc failed");
        return 0;
    }

    bl->prog_h = gl_load_shader("shaders/post_passthrough.vert",
                                "shaders/bloom2_blur_h.frag");
    bl->prog_v = gl_load_shader("shaders/post_passthrough.vert",
                                "shaders/bloom2_blur_v.frag");
    if (!bl->prog_h || !bl->prog_v) {
        LOGW("bloom2: blur fallback shaders missing, safe-mode unavailable");
        return 0;
    }

    bl->ready = 1;
    return 1;
}

void bloom2_blur_destroy(bloom2_blur *bl) {
    bloom2_target_destroy(&bl->scratch);
    gl_delete_shader(bl->prog_h);
    gl_delete_shader(bl->prog_v);
    bl->prog_h = bl->prog_v = 0;
    bl->ready = 0;
}

void bloom2_blur_apply(bloom2_blur *bl, bloom2_target *io, const bloom2_quad *q) {
    if (!bl->ready) return;

    glDisable(GL_BLEND);

    // horizontal: io -> scratch
    bloom2_target_bind(&bl->scratch);
    glUseProgram(bl->prog_h);
    bloom2_target_bind_tex(io, 0);
    gl_set_uniform_int(bl->prog_h, "u_src", 0);
    gl_set_uniform_float(bl->prog_h, "u_texel_x", io->w ? 1.0f/io->w : 0.0f);
    gl_set_uniform_float(bl->prog_h, "u_texel_y", io->h ? 1.0f/io->h : 0.0f);
    upload_taps(bl, bl->prog_h);
    bloom2_quad_draw(q);

    // vertical: scratch -> io
    bloom2_target_bind(io);
    glUseProgram(bl->prog_v);
    bloom2_target_bind_tex(&bl->scratch, 0);
    gl_set_uniform_int(bl->prog_v, "u_src", 0);
    gl_set_uniform_float(bl->prog_v, "u_texel_x", io->w ? 1.0f/io->w : 0.0f);
    gl_set_uniform_float(bl->prog_v, "u_texel_y", io->h ? 1.0f/io->h : 0.0f);
    upload_taps(bl, bl->prog_v);
    bloom2_quad_draw(q);
}
