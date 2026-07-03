#version 330 core
// final composite. additive-blended over the scene framebuffer so we only
// emit the glow scaled by intensity. tonemapping happens elsewhere — we feed
// linear hdr bloom in and let the downstream tonemapper deal with it.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_bloom;
uniform float u_intensity;

void main() {
    vec3 b = texture(u_bloom, v_uv).rgb;
    frag = vec4(b * u_intensity, 1.0);
}
