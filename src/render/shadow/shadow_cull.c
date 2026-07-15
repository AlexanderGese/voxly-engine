#include "shadow_cull.h"

void shadow_cull_build(shadow_cull *sc, const shadow_csm *csm) {
    sc->count = csm->count;
    for (int i = 0; i < csm->count; i++) {
        // frustum_from_matrix extracts the 6 planes from a view*proj. our
        // view_proj is already proj*view so it's exactly what it wants.
        frustum_from_matrix(&sc->f[i], csm->cascade[i].view_proj);
    }
}

// extend an aabb along -light_dir so casters between the light and the slice
// are not culled. cheap and conservative: we just grow the box, we dont skew
// it, which can keep a few extra chunks but never drops a real caster.
static aabb extrude_toward_light(aabb a, vec3 light_dir, float dist) {
    vec3 toward_light = vec3_scale(light_dir, -dist);
    // grow the box to cover both the original and the shifted position
    aabb shifted = aabb_translate(a, toward_light);
    aabb out;
    out.min = vec3_min(a.min, shifted.min);
    out.max = vec3_max(a.max, shifted.max);
    return out;
}

int shadow_cull_test(const shadow_cull *sc, const shadow_csm *csm,
                     int cascade, aabb caster) {
    if (cascade < 0 || cascade >= sc->count) return 0;
    aabb grown = extrude_toward_light(caster, csm->light_dir,
                                      SHADOW_LIGHT_PULLBACK);
    return frustum_contains_aabb(&sc->f[cascade], grown);
}

int shadow_cull_first(const shadow_cull *sc, const shadow_csm *csm,
                      aabb caster) {
    for (int i = 0; i < sc->count; i++) {
        if (shadow_cull_test(sc, csm, i, caster)) return i;
    }
    return -1;
}
