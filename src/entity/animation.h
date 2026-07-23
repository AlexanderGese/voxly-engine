#ifndef ENTITY_ANIMATION_H
#define ENTITY_ANIMATION_H

// per-entity animation state. tracks limb rotation for walk, attack, hurt.

typedef enum {
    ANIM_IDLE = 0,
    ANIM_WALK,
    ANIM_ATTACK,
    ANIM_HURT,
    ANIM_DIE,
} anim_state;

typedef struct {
    anim_state state;
    float      timer;
    float      speed;
    float      limb_angle;   // for walk cycle (sin wave)
    float      arm_angle;    // for attack swing
    float      body_tilt;    // for hurt knockback
} entity_animation;

void  anim_init(entity_animation *a);
void  anim_set_state(entity_animation *a, anim_state s);
void  anim_update(entity_animation *a, float speed, float dt);
float anim_walk_limb(const entity_animation *a);
float anim_attack_arm(const entity_animation *a);
float anim_hurt_tilt(const entity_animation *a);

#endif
