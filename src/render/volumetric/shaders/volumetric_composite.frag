#version 330 core

// composite the blurred god rays over the lit scene. the scatter buffer holds
// rgb = in-scattered light, a = surviving transmittance along the ray. we dim
// the background scene by the transmittance (the medium ate some of it) and add
// the in-scattered light on top. additive, so shafts only ever brighten.
//
// runs at full res; the reduced-res scatter buffer is sampled with LINEAR so
// the upsample is smooth — the whole reason we could afford to march at half
// res in the first place.

in vec2 v_uv;
out vec4 o_color;

uniform sampler2D u_scatter;   // blurred scatter (rgb) + transmittance (a)
uniform sampler2D u_scene;     // the lit scene colour

uniform float u_intensity;     // final fudge multiplier

void main() {
    vec4 s = texture(u_scatter, v_uv);
    vec3 inscatter = s.rgb * u_intensity;
    float transmittance = clamp(s.a, 0.0, 1.0);

    vec3 scene = texture(u_scene, v_uv).rgb;

    // beer-lambert dimming of the background + additive in-scatter. classic
    // single-scattering composite: out = scene * T + inscatter.
    vec3 outc = scene * transmittance + inscatter;

    o_color = vec4(outc, 1.0);
}
