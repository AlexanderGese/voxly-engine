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
}

void player_update(player *p, world *w, float dt) {
    // mouse look
    float mx, my;
    input_mouse_delta(&mx, &my);
    camera_update(&p->cam, mx, my, 0.0025f);
    p->yaw   = p->cam.yaw;
    p->pitch = p->cam.pitch;

    // build move direction in world space from forward/right
    vec3 fwd = camera_forward(&p->cam);
    fwd.y = 0;
    fwd = vec3_normalize(fwd);
    vec3 right = vec3_normalize(vec3_cross(fwd, VEC3_UP));

    vec3 wish = VEC3_ZERO;
    if (input_key_held(GLFW_KEY_W)) wish = vec3_add(wish, fwd);
    if (input_key_held(GLFW_KEY_S)) wish = vec3_sub(wish, fwd);
    if (input_key_held(GLFW_KEY_D)) wish = vec3_add(wish, right);
    if (input_key_held(GLFW_KEY_A)) wish = vec3_sub(wish, right);

    if (vec3_length_sq(wish) > 0.0001f) wish = vec3_normalize(wish);

    float speed = PLAYER_MOVE_SPEED;
    if (input_key_held(GLFW_KEY_LEFT_CONTROL)) speed *= PLAYER_SPRINT_MULT;

    // toggle fly
    if (input_key_pressed(GLFW_KEY_F)) {
        p->flying = !p->flying;
        p->vel.y = 0;
        LOGI("fly: %d", p->flying);
    }

    if (p->flying) {
        p->vel.x = wish.x * speed * 2.0f;
        p->vel.z = wish.z * speed * 2.0f;
        p->vel.y = 0;
        if (input_key_held(GLFW_KEY_SPACE))       p->vel.y =  speed * 2.0f;
        if (input_key_held(GLFW_KEY_LEFT_SHIFT))  p->vel.y = -speed * 2.0f;
    } else {
        p->vel.x = wish.x * speed;
        p->vel.z = wish.z * speed;
        p->vel.y += GRAVITY * dt;
        if (p->vel.y < TERMINAL_VELOCITY) p->vel.y = TERMINAL_VELOCITY;
        if (p->on_ground && input_key_pressed(GLFW_KEY_SPACE)) {
            p->vel.y = PLAYER_JUMP_VEL;
            p->on_ground = 0;
        }
    }

    physics_move_player(p, w, dt);
    player_sync_camera(p);
}
