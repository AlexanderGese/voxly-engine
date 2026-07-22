#version 330 core

// the shared passthrough vert for every screen-space volumetric pass. the
// fullscreen triangle's clip-space positions come straight through; uv is
// derived from them (pos*0.5+0.5), so the c side never ships texcoords.
//
// same trick the ssao + bloom passes use. one vert, three frags.

layout(location = 0) in vec2 a_pos;

out vec2 v_uv;

void main() {
    v_uv = a_pos * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
