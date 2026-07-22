#version 330 core

// one axis of the separable bilateral blur. the c side runs us twice — once
// horizontal, once vertical — by flipping u_dir. the gaussian weights come in
// as a half-kernel (centre + positive side) which we mirror for the negative
// side. the bilateral term reads the scene depth so shafts don't smear across
// silhouettes onto the foreground.
//
// mirrors vol_blur.c's cpu blur_axis().

in vec2 v_uv;
out vec4 o_color;

uniform sampler2D u_scatter;  // the (reduced-res) scatter buffer to blur
uniform sampler2D u_depth;    // scene depth, for the bilateral weight

uniform vec3  u_dir;          // xy = one texel step along the blur axis
uniform int   u_radius;       // taps per side
uniform float u_depth_sigma;  // depth falloff

#define RADIUS_MAX 8
uniform float u_weights[RADIUS_MAX + 1];   // centre at [0]

float depth_weight(float dc, float ds) {
    if (u_depth_sigma <= 0.0) return 1.0;
    float diff = (dc - ds) / u_depth_sigma;
    return exp(-0.5 * diff * diff);
}

void main() {
    vec2 step = u_dir.xy;
    float dc = texture(u_depth, v_uv).r;

    // centre tap
    vec4  acc  = texture(u_scatter, v_uv) * u_weights[0];
    float wsum = u_weights[0];

    int r = min(u_radius, RADIUS_MAX);
    for (int k = 1; k <= r; k++) {
        float gw = u_weights[k];
        vec2 off = step * float(k);

        // + side
        vec2  uvp = v_uv + off;
        float dp  = texture(u_depth, uvp).r;
        float wp  = gw * depth_weight(dc, dp);
        acc  += texture(u_scatter, uvp) * wp;
        wsum += wp;

        // - side
        vec2  uvn = v_uv - off;
        float dn  = texture(u_depth, uvn).r;
        float wn  = gw * depth_weight(dc, dn);
        acc  += texture(u_scatter, uvn) * wn;
        wsum += wn;
    }

    o_color = (wsum > 0.0) ? acc / wsum : texture(u_scatter, v_uv);
}
