#version 330 core

// debug draw vertex shader. one combined view*proj, per-vertex color.
// shared by lines and points; points read u_point_size for gl_PointSize.
// install path is shaders/debugdraw.vert (copied at build, see Makefile).

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_col;   // normalized ubyte rgba

uniform mat4  u_viewproj;
uniform float u_point_size;   // ignored for line draws

out vec4 v_col;

void main() {
    gl_Position  = u_viewproj * vec4(a_pos, 1.0);
    gl_PointSize = u_point_size;
    v_col = a_col;
}
