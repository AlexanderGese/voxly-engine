#version 330 core

// box blur for the ssao occlusion buffer. a single NxN average is enough to
// erase the 4x4 noise tiling pattern. the kernel is tiny so we just loop it
// inline rather than doing two separable passes.
//
// matches ssaox_blur_cpu() in ssao_blur.c (clamp-by-skip at the edges).

in vec2 v_uv;
out float o_occlusion;

uniform sampler2D u_occlusion;
uniform int   u_radius;
uniform float u_texel_x;
uniform float u_texel_y;

void main() {
    float sum = 0.0;
    float n   = 0.0;

    for (int dy = -u_radius; dy <= u_radius; dy++) {
        for (int dx = -u_radius; dx <= u_radius; dx++) {
            vec2 off = vec2(float(dx) * u_texel_x, float(dy) * u_texel_y);
            vec2 uv  = v_uv + off;
            // skip taps that fall outside [0,1] so edges dont darken
            if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) continue;
            sum += texture(u_occlusion, uv).r;
            n   += 1.0;
        }
    }

    o_occlusion = (n > 0.0) ? sum / n : texture(u_occlusion, v_uv).r;
}
