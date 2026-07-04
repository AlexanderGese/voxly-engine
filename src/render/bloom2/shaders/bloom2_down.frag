#version 330 core
// 13-tap downsample from "next gen post processing in cod:aw" (jimenez).
// weighted so the partial-overlap boxes dont introduce stair-step aliasing
// as the pyramid shrinks. src texel size is passed in.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;
uniform float u_texel_x;
uniform float u_texel_y;

void main() {
    float dx = u_texel_x;
    float dy = u_texel_y;

    // inner 4 (the dense box), at +-1 texel
    vec3 a = texture(u_src, v_uv + vec2(-dx, -dy)).rgb;
    vec3 b = texture(u_src, v_uv + vec2( dx, -dy)).rgb;
    vec3 c = texture(u_src, v_uv + vec2(-dx,  dy)).rgb;
    vec3 d = texture(u_src, v_uv + vec2( dx,  dy)).rgb;

    // outer ring at +-2 texels
    vec3 e = texture(u_src, v_uv + vec2(-2.0*dx, -2.0*dy)).rgb;
    vec3 f = texture(u_src, v_uv + vec2( 0.0,    -2.0*dy)).rgb;
    vec3 g = texture(u_src, v_uv + vec2( 2.0*dx, -2.0*dy)).rgb;
    vec3 h = texture(u_src, v_uv + vec2(-2.0*dx,  0.0)).rgb;
    vec3 i = texture(u_src, v_uv).rgb;
    vec3 j = texture(u_src, v_uv + vec2( 2.0*dx,  0.0)).rgb;
    vec3 k = texture(u_src, v_uv + vec2(-2.0*dx,  2.0*dy)).rgb;
    vec3 l = texture(u_src, v_uv + vec2( 0.0,     2.0*dy)).rgb;
    vec3 m = texture(u_src, v_uv + vec2( 2.0*dx,  2.0*dy)).rgb;

    // five overlapping boxes, weights sum to 1
    vec3 o  = (a + b + c + d) * (0.5  * 0.25);
    o += (e + f + i + h) * (0.125 * 0.25);
    o += (f + g + j + i) * (0.125 * 0.25);
    o += (h + i + l + k) * (0.125 * 0.25);
    o += (i + j + m + l) * (0.125 * 0.25);

    frag = vec4(o, 1.0);
}
