#version 330 core

// atlas is white-rgb + coverage-in-alpha. we tint by the per-vertex color and
// multiply alpha by both the glyph coverage and the color's own alpha (so a
// translucent tint fades the whole glyph). discard fully-empty texels to keep
// the depth buffer clean even though we draw with depth test off.

in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_atlas;

out vec4 frag;

void main() {
    float cov = texture(u_atlas, v_uv).a;
    if (cov < 0.02) discard;
    frag = vec4(v_color.rgb, v_color.a * cov);
}
