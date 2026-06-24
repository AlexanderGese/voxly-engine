#include "celestial.h"
#include "sky_math.h"

#include <math.h>

// map hour -> angle around an east/up/west arc. at 6h sun is on the east
// horizon, at 12h straight up, at 18h on the west horizon. nights it's below.
static voxl_sky_body body_from_angle(float ang) {
    voxl_sky_body b;
    // arc in the x (east-west) / y (up) plane. z left at 0, it's a flat ride.
    b.dir.x = cosf(ang);
    b.dir.y = sinf(ang);
    b.dir.z = 0.0f;
    // already unit length from sin/cos, but normalize defensively.
    float len = sqrtf(b.dir.x * b.dir.x + b.dir.y * b.dir.y + b.dir.z * b.dir.z);
    if (len > 0.0f) {
        b.dir.x /= len;
        b.dir.y /= len;
        b.dir.z /= len;
    }
    b.altitude = b.dir.y;
    b.visible = b.dir.y > 0.0f ? 1 : 0;
    return b;
}

voxl_sky_body voxl_sky_sun(float hour) {
    hour = voxl_sky_wrap24(hour);
    // hour 6 -> angle 0 (east horizon), hour 12 -> pi/2 (up), hour 18 -> pi.
    // so angle = (hour - 6)/12 * pi, then it dips negative through the night.
    float ang = (hour - 6.0f) / 12.0f * VOXL_SKY_PI;
    return body_from_angle(ang);
}

voxl_sky_body voxl_sky_moon(float hour) {
    // moon is half a day out of phase with the sun.
    return voxl_sky_sun(voxl_sky_wrap24(hour + 12.0f));
}

float voxl_sky_sun_altitude01(float hour) {
    voxl_sky_body s = voxl_sky_sun(hour);
    return voxl_sky_clampf(s.altitude, 0.0f, 1.0f);
}
