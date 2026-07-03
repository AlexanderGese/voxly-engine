#include "bloom2_tint.h"

#include "../../math/vec3.h"

void bloom2_tint_default(bloom2_tint *t) {
    t->near_tint  = vec3_new(1.0f, 1.0f, 1.0f);
    // far halo leans warm, like cheap camera glass. subtle on purpose.
    t->far_tint   = vec3_new(1.0f, 0.92f, 0.80f);
    t->saturation = 1.0f;
    t->enabled    = 1;
}

vec3 bloom2_tint_for_mip(const bloom2_tint *t, int level, int count) {
    if (!t->enabled || count <= 1)
        return vec3_new(1.0f, 1.0f, 1.0f);

    if (level < 0) level = 0;
    if (level > count - 1) level = count - 1;

    // 0 at the tight mip, 1 at the widest.
    float f = (float)level / (float)(count - 1);
    return vec3_lerp(t->near_tint, t->far_tint, f);
}

vec3 bloom2_tint_saturate(vec3 c, float saturation) {
    // luma-preserving saturation. pull toward/away from grey along the
    // rec.709 luma so brightness doesnt change as we crank color.
    float l = vec3_dot(c, vec3_new(0.2126f, 0.7152f, 0.0722f));
    vec3 grey = vec3_new(l, l, l);
    // lerp(grey, c, sat): sat=0 -> grey, 1 -> original, >1 -> oversaturated
    return vec3_lerp(grey, c, saturation);
}
