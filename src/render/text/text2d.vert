#version 330 core

// screen-space text. positions come in as pixels (top-left origin, y down) and
// we map straight to clip space. one ortho, no matrix uniform — the two screen
// dims are enough and it saves a mat4 upload per flush.

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;   // rgba8, normalized by the vertex format

uniform float u_screen_w;
uniform float u_screen_h;

out vec2 v_uv;
out vec4 v_color;

void main() {
    float x = (a_pos.x / u_screen_w) * 2.0 - 1.0;
    float y = 1.0 - (a_pos.y / u_screen_h) * 2.0;  // flip: pixel y grows down
    gl_Position = vec4(x, y, 0.0, 1.0);
    v_uv    = a_uv;
    v_color = a_color;
}
