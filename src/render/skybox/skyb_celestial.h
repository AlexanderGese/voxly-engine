#ifndef RENDER_SKYBOX_CELESTIAL_H
#define RENDER_SKYBOX_CELESTIAL_H

// sun + moon disc placement for the renderer. this mirrors the world/sky
// celestial model (sun rides a tilted circle) but additionally hands back the
// billboard corners + a tint so the renderer can just draw quads.
//
// the dome is treated as infinitely far: positions are unit dirs scaled by a
// fixed radius. nothing here knows about the camera position, only its look.

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

// four corners of the billboard quad for a body, in world dirs (already
// scaled to `radius`). order is CCW: bl, br, tr, tl. up is world +y biased.
typedef struct {
    vec3 corner[4];
    vec3 uv_sign[4]; // -1/+1 per corner so the shader/cpu can build a disc mask
} skyb_billboard;

// sun disc for the hour. tilt is the orbital tilt in radians (0 = due
// east/west arc). tint comes from the palette sun_tint.
skyb_body skyb_sun(float hour, float tilt, skyb_rgb tint);

// moon, roughly opposite the sun. phase 0..1 dims the disc (new->full->new).
skyb_body skyb_moon(float hour, float tilt, float phase);

// build the billboard for a body. `radius` is the dome radius in world units.
// the quad always faces the viewer along -dir, with a stable up vector.
skyb_billboard skyb_body_billboard(const skyb_body *b, float radius);

// how much direct light this body should contribute, 0..1, for ambient/fog.
float skyb_body_light(const skyb_body *b);

#endif
