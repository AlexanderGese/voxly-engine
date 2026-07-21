#version 330 core
// debug split view: left half of the screen shows the raw exposed-but-untoned
// scene (gamma only), right half shows the full tonemap+grade. drag the seam
// with u_split to eyeball what the grade is actually doing.
// NOTE: same shader set as tonemap_grade.frag; this one is debug-only.

in vec2 v_uv;
out vec4 frag;

uniform sampler2D u_scene;

uniform int   u_curve;
uniform float u_white;
uniform float u_exposure;
uniform float u_inv_gamma;
uniform float u_contrast;
uniform float u_saturation;
uniform vec3  u_white_balance;
uniform float u_middle_grey;
uniform float u_split;        // 0..1 seam position in x

float luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }

float aces(float x) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
float reinhard(float x) { return x / (1.0 + x); }

float curve1(float x) {
    x = max(x, 0.0);
    if (u_curve == 1) return reinhard(x);
    return aces(x);   // good enough for a debug view
}

void main() {
    vec3 hdr = texture(u_scene, v_uv).rgb * u_exposure;

    vec3 c;
    if (v_uv.x < u_split) {
        // raw side: just expose + gamma, no curve, no grade
        c = pow(max(hdr, 0.0), vec3(u_inv_gamma));
    } else {
        vec3 t = vec3(curve1(hdr.r), curve1(hdr.g), curve1(hdr.b));
        t *= u_white_balance;
        t = (t - u_middle_grey) * u_contrast + u_middle_grey;
        float l = luma(t);
        t = mix(vec3(l), t, u_saturation);
        c = pow(clamp(t, 0.0, 1.0), vec3(u_inv_gamma));
    }

    // thin seam line so the boundary is obvious
    float d = abs(v_uv.x - u_split);
    if (d < 0.0015) c = vec3(1.0, 0.9, 0.2);

    frag = vec4(c, 1.0);
}
