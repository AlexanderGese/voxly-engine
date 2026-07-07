#version 330 core
// fxaa prepass: copy the (already tonemapped, ldr) scene through and stash a
// perceptual luma in the alpha channel. the main pass then gets luma for the
// price of the rgb fetch it was making anyway. classic fxaa setup.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_scene;
uniform vec3 u_luma_weights;   // rec.601-ish, matches fxaa_config.h

void main() {
    vec3 c = texture(u_scene, v_uv).rgb;
    // sqrt approximates the perceptual response fxaa expects on linear-ish
    // input without a full gamma curve. keeps thin features from vanishing.
    float luma = sqrt(dot(c, u_luma_weights));
    frag = vec4(c, luma);
}
