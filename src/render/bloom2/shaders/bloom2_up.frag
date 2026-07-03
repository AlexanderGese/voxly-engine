#version 330 core
// 9-tap tent filter upsample. additive-blended (GL_ONE, GL_ONE) into the mip
// above, so this only outputs the new contribution. radius widens the tent.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;
uniform float u_texel_x;
uniform float u_texel_y;
uniform float u_radius;
uniform vec3  u_tint;

void main() {
    float dx = u_texel_x * u_radius;
    float dy = u_texel_y * u_radius;

    // 3x3 tent: corners 1, edges 2, center 4, /16
    vec3 s;
    s  = texture(u_src, v_uv + vec2(-dx, -dy)).rgb * 1.0;
    s += texture(u_src, v_uv + vec2( 0.0, -dy)).rgb * 2.0;
    s += texture(u_src, v_uv + vec2( dx, -dy)).rgb * 1.0;

    s += texture(u_src, v_uv + vec2(-dx,  0.0)).rgb * 2.0;
    s += texture(u_src, v_uv).rgb                   * 4.0;
    s += texture(u_src, v_uv + vec2( dx,  0.0)).rgb * 2.0;

    s += texture(u_src, v_uv + vec2(-dx,  dy)).rgb * 1.0;
    s += texture(u_src, v_uv + vec2( 0.0,  dy)).rgb * 2.0;
    s += texture(u_src, v_uv + vec2( dx,  dy)).rgb * 1.0;

    s *= (1.0 / 16.0);

    // per-mip tint: wider mips lean warm so the big halo isnt the same hue
    // as the tight core. neutral (1,1,1) when tint is disabled.
    s *= u_tint;

    frag = vec4(s, 1.0);
}
