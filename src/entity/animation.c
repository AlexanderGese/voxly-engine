#include "animation.h"

#include <math.h>

void anim_init(entity_animation *a) {
    a->state = ANIM_IDLE;
    a->timer = 0;
    a->speed = 0;
    a->limb_angle = 0;
    a->arm_angle = 0;
    a->body_tilt = 0;
}

void anim_set_state(entity_animation *a, anim_state s) {
    if (a->state == s) return;
    a->state = s;
    a->timer = 0;
}

void anim_update(entity_animation *a, float speed, float dt) {
    a->speed = speed;
    a->timer += dt;

    switch (a->state) {
    case ANIM_IDLE:
        a->limb_angle *= 0.9f;  // decay
        a->arm_angle *= 0.9f;
        a->body_tilt *= 0.9f;
        break;

    case ANIM_WALK:
        a->limb_angle = sinf(a->timer * speed * 8.0f) * 0.6f;
        a->arm_angle  = sinf(a->timer * speed * 8.0f + 3.14f) * 0.4f;
        break;

    case ANIM_ATTACK:
        // quick swing forward then back
        if (a->timer < 0.15f)
            a->arm_angle = -a->timer / 0.15f * 1.5f;
        else if (a->timer < 0.4f)
            a->arm_angle = -1.5f + (a->timer - 0.15f) / 0.25f * 1.5f;
        else
            anim_set_state(a, ANIM_IDLE);
        break;

    case ANIM_HURT:
        a->body_tilt = sinf(a->timer * 20.0f) * 0.3f * fmaxf(0, 1.0f - a->timer * 4.0f);
        if (a->timer > 0.3f) anim_set_state(a, ANIM_IDLE);
        break;

    case ANIM_DIE:
        a->body_tilt = fminf(a->timer * 2.0f, 1.57f); // fall over
        break;
    }
}

float anim_walk_limb(const entity_animation *a) { return a->limb_angle; }
float anim_attack_arm(const entity_animation *a) { return a->arm_angle; }
float anim_hurt_tilt(const entity_animation *a)  { return a->body_tilt; }
