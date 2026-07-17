#version 330 core

// ssao occlusion pass. for each fragment we reconstruct the view-space
// position from depth, orient the sample kernel by the surface normal +
// per-fragment noise rotation, then probe the depth buffer at each sample's
// screen position. occluded samples (scene surface in front of the sample)
// accumulate, range-checked so distant geometry behind a near surface doesnt
// bleed in. output is single channel: 1 = lit, lower = occluded.
//
// canonical source lives in src/render/ssao/ — copied to shaders/ by the
// build. the cpu reference in ssao_sample.c / ssao_compute.c mirrors this.

in vec2 v_uv;
out float o_occlusion;

uniform sampler2D u_depth;
uniform sampler2D u_normal;
uniform sampler2D u_noise;

uniform mat4  u_proj;
uniform mat4  u_inv_proj;

uniform float u_radius;
uniform float u_bias;
uniform float u_power;
uniform float u_strength;
uniform float u_noise_scale_x;
uniform float u_noise_scale_y;

#define KERNEL_MAX 64
uniform vec3 u_kernel[KERNEL_MAX];
uniform int  u_kernel_count;

// reconstruct view-space position from uv + sampled depth via inverse proj.
vec3 view_pos_from_depth(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 vp  = u_inv_proj * ndc;
    return vp.xyz / vp.w;
}

void main() {
    float depth = texture(u_depth, v_uv).r;

    // skip the far plane / sky — nothing to occlude, keep it fully lit.
    if (depth >= 0.99999) { o_occlusion = 1.0; return; }

    vec3 frag = view_pos_from_depth(v_uv, depth);
    vec3 n    = normalize(texture(u_normal, v_uv).xyz * 2.0 - 1.0);

    // per-fragment rotation vector, tiled across the screen.
    vec2 noise_uv = v_uv * vec2(u_noise_scale_x, u_noise_scale_y);
    vec3 rot = texture(u_noise, noise_uv).xyz;

    // gram-schmidt TBN around the normal.
    vec3 t = normalize(rot - n * dot(rot, n));
    vec3 b = cross(n, t);
    mat3 tbn = mat3(t, b, n);

    float occlusion = 0.0;
    int   count = min(u_kernel_count, KERNEL_MAX);

    for (int i = 0; i < count; i++) {
        // sample point in view space
        vec3 sp = frag + (tbn * u_kernel[i]) * u_radius;

        // project to screen
        vec4 clip = u_proj * vec4(sp, 1.0);
        if (clip.w <= 0.0) continue;
        vec3 suv = clip.xyz / clip.w;
        suv = suv * 0.5 + 0.5;
        if (suv.x < 0.0 || suv.x > 1.0 || suv.y < 0.0 || suv.y > 1.0) continue;

        // real scene depth at the sample's screen pos -> its view z
        float sd = texture(u_depth, suv.xy).r;
        float scene_z = view_pos_from_depth(suv.xy, sd).z;

        // occluded if the scene surface is in front of the sample (view z is
        // negative into the screen, so "in front" = larger z), past the bias.
        float occluded = (scene_z >= sp.z + u_bias) ? 1.0 : 0.0;

        // range check: fade samples whose depth gap exceeds the radius.
        float range = smoothstep(0.0, 1.0,
                          u_radius / max(abs(frag.z - scene_z), 1e-4));
        occlusion += occluded * range;
    }

    occlusion = (count > 0) ? occlusion / float(count) : 0.0;
    float lit = clamp(1.0 - occlusion * u_strength, 0.0, 1.0);
    o_occlusion = pow(lit, u_power);
}
