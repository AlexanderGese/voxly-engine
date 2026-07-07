#version 330 core
// passthrough copy. the fallback when fxaa is off or its shaders failed to
// load — we still owe the caller the scene in the destination buffer.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_src;

void main() {
    frag = vec4(texture(u_src, v_uv).rgb, 1.0);
}
