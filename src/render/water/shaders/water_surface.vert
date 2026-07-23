#version 330 core

// water surface vertex stage. the cpu mesh already carries the gerstner
// displacement + analytic normal, so here we just project and pass through.
// the screen-space uv we hand to the fragment shader is what samples the
// reflection / refraction targets.

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec3 u_cam_pos;

out vec3 v_world;
out vec3 v_normal;
out vec2 v_uv;
out vec4 v_clip;     // clip-space pos, so frag can derive screen uv
out vec3 v_view_dir; // surface -> eye

void main() {
    vec4 clip = u_proj * u_view * vec4(a_pos, 1.0);
    gl_Position = clip;

    v_world    = a_pos;
    v_normal   = normalize(a_normal);
    v_uv       = a_uv;
    v_clip     = clip;
    v_view_dir = normalize(u_cam_pos - a_pos);
}
