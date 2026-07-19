#ifndef RENDER_SSR_MATERIAL_H
#define RENDER_SSR_MATERIAL_H

#include "../../math/vec3.h"

// per-fragment surface params ssr cares about. the deferred material buffer
// packs roughness/metalness; if there's no material buffer we fall back to a
// single global default (set whole-scene roughness, no metals). this struct is
// what the resolve actually consumes after unpacking.

typedef struct {
    float roughness;   // 0 mirror .. 1 fully diffuse
    float metalness;   // 0 dielectric .. 1 metal
    vec3  albedo;      // surface base color, feeds metal f0
} ssrx_material;

// a sensible default surface when no material buffer is bound: a slightly rough
// dielectric. wet-stone-ish, which is the main thing we reflect anyway.
ssrx_material ssrx_material_default(void);

// unpack a material from the buffer's raw channels. r=roughness, g=metalness;
// albedo arrives separately (usually from the color buffer). values clamped.
ssrx_material ssrx_material_unpack(float r, float g, vec3 albedo);

// roughness gates whether ssr even bothers: very rough surfaces scatter the
// reflection into noise the single-ray march cant represent. returns a 0..1
// weight that tapers ssr off past `cutoff` roughness.
float ssrx_material_reflectivity(const ssrx_material *m, float cutoff);

#endif
