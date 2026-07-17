#include "ssao_sample.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// classic smoothstep, same shape as glsl's.
static float smoothstepf(float e0, float e1, float x) {
    if (e0 == e1) return x < e0 ? 0.0f : 1.0f;
    float t = clampf((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

int ssaox_project(mat4 proj, vec3 view_pos, vec2 *out_uv, float *out_depth) {
    // multiply by proj manually so we keep w (mat4_mul_vec3 assumes w=1 and
    // drops the perspective divide info we need here).
    const float (*m)[4] = proj.m;  // column major: m[col][row]
    float x = view_pos.x, y = view_pos.y, z = view_pos.z;

    float cx = m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0];
    float cy = m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1];
    float cz = m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2];
    float cw = m[0][3]*x + m[1][3]*y + m[2][3]*z + m[3][3];

    if (cw <= 1e-6f) return 0;   // at or behind the near plane, useless

    float inv = 1.0f / cw;
    float ndc_x = cx * inv;
    float ndc_y = cy * inv;
    float ndc_z = cz * inv;

    out_uv->x = ndc_x * 0.5f + 0.5f;
    out_uv->y = ndc_y * 0.5f + 0.5f;
    if (out_depth) *out_depth = ndc_z * 0.5f + 0.5f;
    return 1;
}

mat4 ssaox_tbn(vec3 normal, vec3 rotation) {
    vec3 n = vec3_normalize(normal);

    // gram-schmidt: remove the component of `rotation` along the normal so
    // the tangent ends up in the surface plane, then re-normalize.
    float d = vec3_dot(rotation, n);
    vec3 t = vec3_sub(rotation, vec3_scale(n, d));
    float tl = vec3_length(t);
    if (tl < 1e-5f) {
        // rotation was (near) parallel to the normal — pick any tangent.
        vec3 ref = fabsf(n.y) < 0.99f ? VEC3_UP : VEC3_RIGHT;
        t = vec3_cross(ref, n);
    }
    t = vec3_normalize(t);
    vec3 b = vec3_cross(n, t);

    // columns are the basis vectors (tangent, bitangent, normal).
    mat4 r = mat4_identity();
    r.m[0][0] = t.x; r.m[0][1] = t.y; r.m[0][2] = t.z;
    r.m[1][0] = b.x; r.m[1][1] = b.y; r.m[1][2] = b.z;
    r.m[2][0] = n.x; r.m[2][1] = n.y; r.m[2][2] = n.z;
    return r;
}

float ssaox_range_check(float radius, float frag_view_z, float sample_view_z) {
    // view-space z is negative going into the screen; use magnitude of the
    // difference. fade from full weight at 0 to zero weight at radius.
    float diff = fabsf(frag_view_z - sample_view_z);
    return 1.0f - smoothstepf(radius * 0.5f, radius, diff);
}

float ssaox_occlusion_term(float radius, float bias,
                           float sample_view_z, float scene_view_z) {
    // sample is occluded if the real scene surface at that screen pos is
    // *closer to the camera* than the sample point (i.e. in front of it),
    // by more than the bias. remember view z is negative into the screen,
    // so "closer" means a larger (less negative) z.
    float occluded = (scene_view_z >= sample_view_z + bias) ? 1.0f : 0.0f;
    float range = ssaox_range_check(radius, sample_view_z, scene_view_z);
    return occluded * range;
}
