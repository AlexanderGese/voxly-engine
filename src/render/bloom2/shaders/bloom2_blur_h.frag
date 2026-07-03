#version 330 core
// horizontal pass of the separable gaussian fallback. weights/offsets come
// from the cpu (linear-packed) so we only do ceil(N/2) fetches.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;
uniform float u_texel_x;
uniform float u_texel_y;

uniform int   u_taps;
uniform float u_weight[8];
uniform float u_offset[8];

void main() {
    // center tap (offset 0) is index 0, full weight, no mirror.
    vec3 acc = texture(u_src, v_uv).rgb * u_weight[0];

    for (int i = 1; i < u_taps; i++) {
        float ox = u_offset[i] * u_texel_x;
        acc += texture(u_src, v_uv + vec2( ox, 0.0)).rgb * u_weight[i];
        acc += texture(u_src, v_uv + vec2(-ox, 0.0)).rgb * u_weight[i];
    }
    frag = vec4(acc, 1.0);
}
