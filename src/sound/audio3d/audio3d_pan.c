#include "audio3d_pan.h"

#include <math.h>

#ifndef AUDIO3D_HALF_PI
#define AUDIO3D_HALF_PI 1.57079632679f
#endif

audio3d_pan audio3d_pan_from_azimuth(float azimuth, float gain, float width) {
    if (azimuth < -1.0f) azimuth = -1.0f;
    if (azimuth >  1.0f) azimuth =  1.0f;
    azimuth *= width;

    // map -1..1 to 0..pi/2 and use sin/cos so left^2 + right^2 == 1.
    // this is the equal-power pan; -3db in the centre, which is what you want.
    float t = (azimuth * 0.5f + 0.5f) * AUDIO3D_HALF_PI;
    audio3d_pan p;
    p.left  = cosf(t) * gain;
    p.right = sinf(t) * gain;
    return p;
}

audio3d_pan audio3d_pan_spatial(const audio3d_listener *l, vec3 src_pos,
                                float gain, float width) {
    vec3 local = audio3d_listener_local(l, src_pos);

    // azimuth in the horizontal (right/fwd) plane. atan2 keeps it stable even
    // when the source is dead ahead and local.right is ~0.
    float az_rad = atan2f(local.x, local.z >= 0.0f ? local.z : 0.0f);
    // ^ we only feed the forward component when it's in front; behind, fall
    // back to a flat magnitude split so it doesnt swing wildly.
    float side_mag = sqrtf(local.x * local.x + local.z * local.z);
    float azimuth;
    if (side_mag < AUDIO3D_EPS) {
        azimuth = 0.0f;             // right on the axis, centre it
    } else {
        azimuth = local.x / side_mag;   // -1..1, sin of the bearing
    }
    (void)az_rad;

    audio3d_pan p = audio3d_pan_from_azimuth(azimuth, gain, width);

    // head-shadow: sources behind the listener (local.z < 0) lose a touch of
    // overall energy. cheap psychoacoustic fudge, sounds better than nothing.
    if (local.z < 0.0f) {
        float behind = -local.z / (fabsf(local.x) + fabsf(local.z) + AUDIO3D_EPS);
        float shadow = 1.0f - 0.25f * behind;   // up to -25% dead behind
        p.left  *= shadow;
        p.right *= shadow;
    }
    return p;
}
