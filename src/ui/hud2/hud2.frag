#version 330 core

// dead simple: just emit the interpolated vertex color. all the hud styling
// (panels, borders, fades) is baked into the per-vertex alpha by the cpu-side
// batcher, so the fragment stage has nothing clever to do. drawn with depth
// test off and standard alpha blending.

in vec4 v_color;

out vec4 frag;

void main() {
    // drop fully transparent fragments so overlapping edge bands dont rack up
    // pointless blend ops.
    if (v_color.a < 0.003) discard;
    frag = v_color;
}
