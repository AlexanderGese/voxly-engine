#version 330 core
// bright pass with soft knee. mirror of unreal's threshold curve so the
// transition into bloom doesnt pop as a hard edge.
// NOTE: build copies these into shaders/ next to the other post shaders;
// they live here so the bloom2 module is self-contained.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_scene;

// knee curve precomputed on the cpu (see bloom2_params_knee_curve)
uniform float u_knee_x;   // threshold
uniform float u_knee_y;   // threshold - knee
uniform float u_knee_z;   // 2 * knee
uniform float u_knee_w;   // 0.25 / knee
uniform float u_clamp;    // firefly luma clamp
uniform float u_texel_x;
uniform float u_texel_y;

float luma(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    // 4-tap box prefilter to kill some aliasing before we threshold. half a
    // texel diagonal offset, bilinear does the averaging for us.
    vec2 o = vec2(u_texel_x, u_texel_y) * 0.5;
    vec3 c = texture(u_scene, v_uv + vec2(-o.x, -o.y)).rgb
           + texture(u_scene, v_uv + vec2( o.x, -o.y)).rgb
           + texture(u_scene, v_uv + vec2(-o.x,  o.y)).rgb
           + texture(u_scene, v_uv + vec2( o.x,  o.y)).rgb;
    c *= 0.25;

    float br = luma(c);

    // soft knee: smooth ramp between (threshold-knee) and (threshold+knee)
    float soft = br - u_knee_y;
    soft = clamp(soft, 0.0, u_knee_z);
    soft = soft * soft * u_knee_w;
    float contrib = max(soft, br - u_knee_x);
    contrib /= max(br, 1e-5);

    c *= contrib;

    // firefly clamp on the output luma so a stray inf cant nuke the frame
    float ol = luma(c);
    if (ol > u_clamp) c *= (u_clamp / ol);

    frag = vec4(c, 1.0);
}
