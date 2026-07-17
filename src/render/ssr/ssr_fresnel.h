#ifndef RENDER_SSR_FRESNEL_H
#define RENDER_SSR_FRESNEL_H

#include "../../math/vec3.h"

// schlick fresnel. reflections are strongest at grazing angles, weakest head
// on — that head-on minimum is f0. the whole "wet road looks like a mirror in
// the distance but not under your feet" effect lives in this one curve.

// scalar schlick. cos_theta is dot(normal, view_to_surface)... we actually
// feed it dot(-view_dir, normal) clamped to [0,1]. f0 in [0,1].
float ssrx_fresnel_schlick(float cos_theta, float f0);

// same, clamped to the configured [min,max] band so a pathological f0 cant
// make reflections fully opaque (or vanish entirely).
float ssrx_fresnel_clamped(float cos_theta, float f0);

// build the effective f0 for a fragment: dielectric baseline lerped toward the
// surface albedo by metalness, the usual metallic-workflow trick. returns a
// scalar — ssr blends a single reflection color so we collapse to luminance-ish
// weight rather than carrying an rgb f0.
float ssrx_fresnel_f0_metal(float base_f0, vec3 albedo, float metalness);

// roughness damps fresnel a touch (rough metal is less mirror-like at grazing).
// gltf-style f90 rolloff, cheap approximation.
float ssrx_fresnel_roughness(float cos_theta, float f0, float roughness);

#endif
