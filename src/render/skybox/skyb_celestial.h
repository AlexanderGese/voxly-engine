#ifndef RENDER_SKYBOX_CELESTIAL_H
#define RENDER_SKYBOX_CELESTIAL_H
#include "skyb_common.h"
#include "../../math/mat4.h"
typedef enum {
    SKYB_BODY_SUN = 0,
    SKYB_BODY_MOON = 1,
} skyb_body_kind;
typedef struct {
    skyb_body_kind kind;
    vec3   dir;        // unit dir from viewer toward the body
    float  altitude;   // sin of angle above horizon, -1..1
    float  visible01;  // 0 below horizon, smooth fade at the edge, 1 high up
    float  angular_sz; // apparent radius in radians
    skyb_rgb tint;     // color the disc gets multiplied by
} skyb_body;
#endif
