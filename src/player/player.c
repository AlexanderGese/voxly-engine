#include "player.h"
#include "physics.h"
#include "../input/input.h"
#include "../config.h"
#include "../util/log.h"
#include <GLFW/glfw3.h>
#include <math.h>
void player_init(player *p, vec3 spawn) {
    p->pos = spawn;
    p->vel = VEC3_ZERO;
    p->on_ground = 0;
    p->flying = 0;
    p->yaw = 0;
    p->pitch = 0;
    camera_init(&p->cam);
    p->cam.pos = spawn;
}

void player_sync_camera(player *p) {
    p->cam.pos   = (vec3){p->pos.x, p->pos.y + PLAYER_EYE_HEIGHT, p->pos.z};
p->cam.yaw   = p->yaw;
p->cam.pitch = p->pitch;
