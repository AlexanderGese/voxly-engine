#version 330 core

// hud2 2d color batch. vertices arrive in pixels (top-left origin, y down) and
// map straight to clip space — same trick as the text shader, no matrix upload.
// color is interleaved per-vertex; the batcher packs everything into one buffer
// so this runs once per flush.

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec4 a_color;

uniform float u_sw;
uniform float u_sh;

out vec4 v_color;

void main() {
    float x = (a_pos.x / u_sw) * 2.0 - 1.0;
    float y = 1.0 - (a_pos.y / u_sh) * 2.0;   // flip: pixel y grows down
    gl_Position = vec4(x, y, 0.0, 1.0);
    v_color = a_color;
}
