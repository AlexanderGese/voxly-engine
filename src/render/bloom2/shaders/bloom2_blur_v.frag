#version 330 core
// vertical pass of the separable gaussian fallback. same as the horizontal
// one but stepping along y.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;
uniform float u_texel_x;
uniform float u_texel_y;

uniform int   u_taps;
uniform float u_weight[8];
uniform float u_offset[8];

void main() {
    vec3 acc = texture(u_src, v_uv).rgb * u_weight[0];

    for (int i = 1; i < u_taps; i++) {
        float oy = u_offset[i] * u_texel_y;
        acc += texture(u_src, v_uv + vec2(0.0,  oy)).rgb * u_weight[i];
        acc += texture(u_src, v_uv + vec2(0.0, -oy)).rgb * u_weight[i];
    }
    frag = vec4(acc, 1.0);
}
