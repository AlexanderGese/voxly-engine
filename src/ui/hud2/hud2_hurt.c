#include "hud2_hurt.h"
#include "hud2_anim.h"
#include <math.h>

float hud2_hurt_dir_from_world(vec3 player_pos, float yaw, vec3 source_pos) {
    // vector from player to whatever hit us, flattened to the xz plane. the
    // y component doesnt matter for an edge flash.
    float dx = source_pos.x - player_pos.x;
    float dz = source_pos.z - player_pos.z;

    if (dx * dx + dz * dz < 1e-6f)
        return 0.0f;   // on top of us, just flash front

    // world angle of the source, then subtract the player's facing so we get
    // it relative to where we're looking. yaw convention here: 0 looks toward
    // +z, increasing yaw turns right. atan2(dx,dz) matches that.
    float world_ang = atan2f(dx, dz);
    float rel = world_ang - yaw;

    // wrap to -pi..pi
    while (rel >  3.14159265f) rel -= 6.28318530f;
    while (rel < -3.14159265f) rel += 6.28318530f;
    return rel;
}

float hud2_hurt_strength(int damage) {
    if (damage <= 0) return 0.0f;
    // soft saturating curve: 1 dmg -> ~0.3, 6 dmg -> ~0.85, asymptote at 1.
    float d = (float)damage;
    float s = 1.0f - expf(-d * 0.32f);
    return hud2_clampf(s, 0.15f, 1.0f);
}

float hud2_hurt_dir_environment(void) {
    // pi/2 in our side mapping is "bottom" -> reads as ground/fall damage.
    return 1.5707963f;
}
