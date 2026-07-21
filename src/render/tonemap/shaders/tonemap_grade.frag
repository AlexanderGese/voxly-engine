#version 330 core
// hdr tonemap + color grade + lut, the final ldr write. mirrors the cpu code
// in tonemap_curve.c / tonemap_grade.c so previews match the real frame.
// NOTE: build copies these into shaders/ next to the other post shaders.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_scene;
uniform sampler3D u_lut;

// curve
uniform int   u_curve;       // TONEMAP_CURVE_*
uniform float u_white;
uniform float u_exposure;
uniform float u_inv_gamma;
uniform int   u_enabled;

// grade
uniform float u_contrast;
uniform float u_saturation;
uniform vec3  u_white_balance;
uniform vec3  u_lift;
uniform vec3  u_grade_gamma;  // exponent is 1/this, per channel
uniform vec3  u_gain;
uniform float u_middle_grey;

// lut
uniform float u_lut_weight;
uniform float u_lut_size;

float luma(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

float reinhard(float x)            { return x / (1.0 + x); }
float reinhard_ext(float x, float w) {
    if (w <= 0.0) return reinhard(x);
    float w2 = w * w;
    return clamp((x * (1.0 + x / w2)) / (1.0 + x), 0.0, 1.0);
}
float aces(float x) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
float hable_partial(float x) {
    const float a = 0.15, b = 0.50, c = 0.10, d = 0.20, e = 0.02, f = 0.30;
    return ((x * (a * x + c * b) + d * e) /
            (x * (a * x + b) + d * f)) - e / f;
}
float filmic(float x) {
    float norm = hable_partial(11.2);
    return clamp(hable_partial(x) / max(norm, 1e-6), 0.0, 1.0);
}

float curve1(float x) {
    x = max(x, 0.0);
    if (u_curve == 0) return clamp(x, 0.0, 1.0);   // linear
    if (u_curve == 1) return reinhard(x);
    if (u_curve == 2) return reinhard_ext(x, u_white);
    if (u_curve == 4) return filmic(x);
    return aces(x);                                 // 3 / default
}

vec3 apply_curve(vec3 c) {
    return vec3(curve1(c.r), curve1(c.g), curve1(c.b));
}

// lift / gamma / gain, asc-cdl-ish, per channel
vec3 lgg(vec3 c) {
    vec3 v = max(c * u_gain + u_lift, vec3(0.0));
    return pow(v, vec3(1.0) / u_grade_gamma);
}

vec3 grade(vec3 c) {
    c *= u_white_balance;
    c = lgg(c);
    c = (c - u_middle_grey) * u_contrast + u_middle_grey;
    float l = luma(c);
    c = mix(vec3(l), c, u_saturation);
    return clamp(c, 0.0, 1.0);
}

void main() {
    vec3 hdr = texture(u_scene, v_uv).rgb;

    if (u_enabled == 0) {
        // bypass: straight gamma encode of the raw scene, no grade.
        frag = vec4(pow(max(hdr, 0.0), vec3(u_inv_gamma)), 1.0);
        return;
    }

    // expose then tonemap into [0,1]
    vec3 c = apply_curve(hdr * u_exposure);

    // creative grade
    c = grade(c);

    // 3d lut: nudge sampling into the texel centers so the edges dont bias.
    if (u_lut_weight > 0.0 && u_lut_size >= 2.0) {
        float scale = (u_lut_size - 1.0) / u_lut_size;
        float offset = 0.5 / u_lut_size;
        vec3 luc = texture(u_lut, c * scale + offset).rgb;
        c = mix(c, luc, u_lut_weight);
    }

    // output gamma encode (cheap srgb approximation)
    c = pow(max(c, 0.0), vec3(u_inv_gamma));
    frag = vec4(c, 1.0);
}
