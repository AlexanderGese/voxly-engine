#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include "../math/vec3.h"
#include "../render/camera.h"
#include "../world/world.h"

// aabb for player. position is the feet.
typedef struct {
    vec3   pos;
    vec3   vel;
    int    on_ground;
    int    flying;
    float  yaw;
    float  pitch;

    camera cam;
} player;

void player_init(player *p, vec3 spawn);
void player_update(player *p, world *w, float dt);
void player_sync_camera(player *p);

#endif
