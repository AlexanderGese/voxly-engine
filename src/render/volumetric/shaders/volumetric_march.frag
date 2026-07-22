#version 330 core

// the raymarch. for each (reduced-res) pixel we reconstruct the world position
// the depth buffer says the scene surface is at, walk a ray from the camera to
// it, and at every step test the sun's shadow map: lit steps add in-scattered
// light weighted by the henyey-greenstein phase and the accumulated
// transmittance; shadowed steps add nothing. the first step is dithered per
// pixel so the discrete march turns into noise the blur eats.
//
// this mirrors the cpu reference in vol_raymarch.c / vol_phase.c line for line.
// if you change the integration here, change it there too.

in vec2 v_uv;
out vec4 o_scatter;          // rgb = in-scattered light, a = transmittance

uniform sampler2D u_depth;   // scene depth (full res, we just sample it)
uniform sampler2D u_shadow;  // sun shadow map (depth from the light)
uniform sampler2D u_dither;  // bayer start-offset tile

uniform mat4  u_inv_vp;      // inverse view-projection (world reconstruction)
uniform mat4  u_light_vp;    // sun view-projection (shadow lookup)

uniform vec3  u_to_sun;      // normalized, toward the sun
uniform vec3  u_sun_color;   // light colour, already * intensity * strength

uniform float u_g;           // hg anisotropy
uniform float u_scatter;     // scattering coeff per world unit
uniform float u_extinct;     // extinction coeff per world unit
uniform int   u_steps;       // samples along the ray
uniform float u_max_dist;    // march cutoff, world units

uniform vec3  u_dither_scale; // xy = uv tiling, z = per-frame phase

const float INV_4PI = 0.07957747;

// world position from this pixel's uv + sampled depth.
vec3 world_from_depth(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 wp  = u_inv_vp * ndc;
    return wp.xyz / wp.w;
}

// henyey-greenstein phase, normalized to integrate to 1 over the sphere.
float phase_hg(float g, float cos_theta) {
    float g2 = g * g;
    float d  = 1.0 + g2 - 2.0 * g * cos_theta;
    d = max(d, 1e-6);
    return INV_4PI * (1.0 - g2) / (d * sqrt(d));
}

// 1 if the world point is lit by the sun, 0 if the shadow map says occluded.
float sun_visible(vec3 world) {
    vec4 lp = u_light_vp * vec4(world, 1.0);
    vec3 proj = lp.xyz / lp.w;
    proj = proj * 0.5 + 0.5;                 // -> [0,1] shadow-map space
    // outside the light frustum: treat as lit (no shadow info out there).
    if (proj.x < 0.0 || proj.x > 1.0 ||
        proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0) return 1.0;
    float closest = texture(u_shadow, proj.xy).r;
    float bias = 0.0015;                      // peter-panning vs acne tradeoff
    return (proj.z - bias > closest) ? 0.0 : 1.0;
}

void main() {
    // camera origin: unproject the near-plane centre of this pixel. depth 0 is
    // the near plane in our [0,1] convention.
    vec3 origin = world_from_depth(v_uv, 0.0);

    float depth = texture(u_depth, v_uv).r;
    vec3 target = world_from_depth(v_uv, depth);

    vec3 delta = target - origin;
    float dist = length(delta);
    if (dist < 1e-4) { o_scatter = vec4(0.0, 0.0, 0.0, 1.0); return; }
    dist = min(dist, u_max_dist);
    vec3 dir = delta / length(delta);

    int steps = clamp(u_steps, 1, 256);
    float dx = dist / float(steps);

    // dithered start offset in [0,1). the z phase rotates the tile slightly per
    // frame so a static camera doesn't show a fixed screen-door.
    vec2 dither_uv = v_uv * u_dither_scale.xy + u_dither_scale.z * 0.013;
    float offset = texture(u_dither, dither_uv).r;

    float cos_vl = clamp(dot(dir, u_to_sun), -1.0, 1.0);
    float phase  = phase_hg(u_g, cos_vl);
    float step_t = exp(-u_extinct * dx);     // per-step transmittance, constant

    vec3  accum = vec3(0.0);
    float transmittance = 1.0;

    for (int i = 0; i < steps; i++) {
        float t = (float(i) + offset) * dx;
        if (t > dist) break;
        vec3 p = origin + dir * t;

        float lit = sun_visible(p);
        if (lit > 0.5) {
            float w = transmittance * u_scatter * phase * dx;
            accum += u_sun_color * w;
        }
        transmittance *= step_t;
        if (transmittance < 0.003) break;    // tail is negligible, bail
    }

    o_scatter = vec4(accum, transmittance);
}
