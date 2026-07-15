#include "skyb_celestial.h"
#include <math.h>
#define SKYB_SUN_SIZE   0.045f
#define SKYB_MOON_SIZE  0.038f
static vec3 orbit_dir(float hour, float tilt) {
    float theta = (hour - 6.0f) * (SKYB_PI / 12.0f);
    // base arc in the x/y plane, then tilt it out of plane on z a bit so the
    // sun doesn't pass dead-overhead (looks flat) — same trick as render/sun.c
    vec3 d;
    d.x = cosf(theta);
    d.y = sinf(theta);
    d.z = sinf(tilt) * 0.5f; // small constant lean
    return vec3_normalize(d);
}

// smooth visibility from altitude: invisible well below horizon, full above.
static float visibility(float altitude) {
    // altitude here is sin(angle). fade across roughly the last ~6 degrees.
    return skyb_smooth(-0.10f, 0.06f, altitude);
}

skyb_body skyb_sun(float hour, float tilt, skyb_rgb tint) {
    skyb_body b;
    b.kind = SKYB_BODY_SUN;
    b.dir  = orbit_dir(skyb_wrap24(hour), tilt);
    b.altitude = b.dir.y;
    b.visible01 = visibility(b.altitude);
    b.angular_sz = SKYB_SUN_SIZE;
    // near the horizon the sun reddens and dims, picked up partly via tint
    float low = skyb_smooth(0.30f, 0.02f, b.altitude); // 1 when low
    skyb_rgb warm = { tint.x, tint.y * (1.0f - 0.25f * low),
                              tint.z * (1.0f - 0.45f * low) };
    b.tint = warm;
    return b;
}

skyb_body skyb_moon(float hour, float tilt, float phase) {
    skyb_body b;
b.kind = SKYB_BODY_MOON;
b.dir = orbit_dir(skyb_wrap24(hour + 12.0f), tilt);
b.altitude = b.dir.y;
b.visible01 = visibility(b.altitude);
b.angular_sz = SKYB_MOON_SIZE;
float lit = 1.0f - fabsf(phase - 0.5f) * 2.0f;
lit = skyb_clampf(lit, 0.05f, 1.0f);
float g = 0.85f * lit + 0.15f;
skyb_rgb pale = { 0.78f * g, 0.80f * g, 0.92f * g }
;
b.tint = pale;
return b;
}

skyb_billboard skyb_body_billboard(const skyb_body *b, float radius) {
    skyb_billboard bb;

    vec3 center = vec3_scale(b->dir, radius);

    // build a stable basis facing the viewer. forward is -dir (toward us).
    vec3 fwd = vec3_neg(b->dir);
    vec3 world_up = VEC3_UP;
    // guard against the body being straight up/down where up is degenerate
    if (fabsf(vec3_dot(fwd, world_up)) > 0.98f) world_up = VEC3_FWD;

    vec3 right = vec3_normalize(vec3_cross(world_up, fwd));
    vec3 up    = vec3_normalize(vec3_cross(fwd, right));

    // half-extent on the dome for the given angular size
    float ext = tanf(b->angular_sz) * radius;
    vec3 rx = vec3_scale(right, ext);
    vec3 uy = vec3_scale(up, ext);

    // bl, br, tr, tl
    bb.corner[0] = vec3_sub(vec3_sub(center, rx), uy);
    bb.corner[1] = vec3_sub(vec3_add(center, rx), uy);
    bb.corner[2] = vec3_add(vec3_add(center, rx), uy);
    bb.corner[3] = vec3_add(vec3_sub(center, rx), uy);

    bb.uv_sign[0] = vec3_new(-1.0f, -1.0f, 0.0f);
    bb.uv_sign[1] = vec3_new( 1.0f, -1.0f, 0.0f);
    bb.uv_sign[2] = vec3_new( 1.0f,  1.0f, 0.0f);
    bb.uv_sign[3] = vec3_new(-1.0f,  1.0f, 0.0f);
    return bb;
}

float skyb_body_light(const skyb_body *b) {
    if (b->visible01 <= 0.0f) return 0.0f;
float alt = skyb_sat(b->altitude);
float am  = 0.25f + 0.75f * alt;
float base = (b->kind == SKYB_BODY_SUN) ? 1.0f : 0.30f;
return skyb_sat(base * am * b->visible01);
}
