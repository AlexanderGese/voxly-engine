#include "water_plane.h"
#include "water_config.h"

water_plane water_plane_make(float y) {
    water_plane wp;
    wp.height = y;

    // the surface itself sits a touch below the block top so the clip plane
    // doesnt fight the water quad. normal points up for the "keep below" test
    // and down for "keep above". distance term puts the plane at y.
    float py = y - WATER_PLANE_BIAS;

    wp.up.n   = (vec3){0, 1, 0};
    wp.up.d   = -py;            // n.p + d == p.y - py
    wp.down.n = (vec3){0, -1, 0};
    wp.down.d =  py;            // n.p + d == -p.y + py

    return wp;
}

int water_plane_above(const water_plane *wp, vec3 p) {
    return p.y >= wp->height;
}

vec3 water_plane_reflect_point(const water_plane *wp, vec3 p) {
    // mirror across the horizontal plane: y -> 2h - y
    vec3 r = p;
    r.y = 2.0f * wp->height - p.y;
    return r;
}

vec3 water_plane_reflect_dir(const water_plane *wp, vec3 d) {
    (void)wp;
    vec3 r = d;
    r.y = -d.y;
    return r;
}

vec4 water_plane_clip_eq(const water_plane *wp, int pass_reflection) {
    // gl clip plane wants ax+by+cz+d >= 0 to be kept. for reflection we keep
    // everything above the surface; for refraction everything below.
    const plane *src = pass_reflection ? &wp->down : &wp->up;
    return (vec4){ src->n.x, src->n.y, src->n.z, src->d };
}
