#version 330 core

// the surface fragment stage. samples the mirrored reflection and the clipped
// refraction targets, distorts both by the fine ripple normal, blends them by
// a schlick fresnel term and tints by depth. caustics get added underneath but
// only show through where the water is shallow.

in vec3 v_world;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_clip;
in vec3 v_view_dir;

uniform sampler2D u_reflection;
uniform sampler2D u_refraction;
uniform sampler2D u_refract_depth;
uniform sampler2D u_caustics;

uniform float u_time;
uniform float u_distort;
uniform float u_f0;
uniform float u_fresnel_power;
uniform float u_tint_depth;
uniform vec3  u_tint_shallow;
uniform vec3  u_tint_deep;

out vec4 frag;

// fine ripple normal, layered on top of the coarse mesh normal. two scrolling
// sine fields crossed, cheap and tileable.
vec3 ripple_normal(vec2 uv) {
    float a = sin(uv.x * 18.0 + u_time * 1.7) * 0.5
            + sin((uv.x + uv.y) * 11.0 - u_time * 1.3) * 0.5;
    float b = cos(uv.y * 16.0 - u_time * 1.1) * 0.5
            + cos((uv.x - uv.y) * 13.0 + u_time * 0.9) * 0.5;
    // small perturbation around straight up
    return normalize(vec3(a * 0.06, 1.0, b * 0.06));
}

void main() {
    // screen-space uv from clip pos (perspective divide -> ndc -> 0..1)
    vec2 ndc = v_clip.xy / v_clip.w;
    vec2 suv = ndc * 0.5 + 0.5;

    // combine the coarse wave normal with the fine ripple
    vec3 fine = ripple_normal(v_uv);
    vec3 n = normalize(v_normal + vec3(fine.x, 0.0, fine.z));

    // distort the sample uvs by the perturbed normal. reflection samples from
    // the y-flipped target so it reads right-side-up.
    vec2 distort = n.xz * u_distort;

    vec2 refl_uv = vec2(suv.x, 1.0 - suv.y) + distort;
    vec2 refr_uv = suv - distort;
    refl_uv = clamp(refl_uv, 0.001, 0.999);
    refr_uv = clamp(refr_uv, 0.001, 0.999);

    vec3 reflection = texture(u_reflection, refl_uv).rgb;
    vec3 refraction = texture(u_refraction, refr_uv).rgb;

    // water depth from the refraction depth buffer. linearise roughly: we only
    // need a relative measure for tint + edge fade, so a cheap remap is fine.
    float scene_d = texture(u_refract_depth, refr_uv).r;
    float surf_d  = gl_FragCoord.z;
    float depth   = max(0.0, (scene_d - surf_d)) * u_tint_depth * 4.0;

    // tint the refraction by depth so deep water goes murky
    float t = clamp(depth / u_tint_depth, 0.0, 1.0);
    t = t * t * (3.0 - 2.0 * t);
    vec3 tint = mix(u_tint_shallow, u_tint_deep, t);
    refraction = mix(refraction, tint, t * 0.85);

    // caustics: bright only in shallow water, scrolled by time
    float caust = texture(u_caustics, v_uv * 2.0 + vec2(u_time * 0.03)).r;
    refraction += caust * (1.0 - t) * 0.25 * vec3(0.8, 0.95, 1.0);

    // schlick fresnel
    float cosv = clamp(dot(v_view_dir, n), 0.0, 1.0);
    float f = u_f0 + (1.0 - u_f0) * pow(1.0 - cosv, u_fresnel_power);
    f = clamp(f, 0.05, 0.85);

    vec3 col = mix(refraction, reflection, f);

    // a touch of specular off the ripple for sun glints
    vec3 light_dir = normalize(vec3(0.4, 0.8, 0.3));
    vec3 half_v = normalize(light_dir + v_view_dir);
    float spec = pow(max(dot(n, half_v), 0.0), 80.0);
    col += spec * 0.4;

    // soften the very edge where water meets land so there's no hard seam
    float edge = clamp(depth * 2.0, 0.0, 1.0);
    float alpha = mix(0.55, 0.92, edge);

    frag = vec4(col, alpha);
}
