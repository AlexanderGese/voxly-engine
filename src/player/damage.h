#ifndef PLAYER_DAMAGE_H
#define PLAYER_DAMAGE_H

// simple damage system for future enemies. right now only fall damage works.

#include "player.h"

#define PLAYER_MAX_HP 20

typedef struct {
    int   hp;
    float prev_vel_y;
    float hit_flash;   // seconds remaining for red overlay
} damage;

void damage_init(damage *d);
void damage_update(damage *d, const player *p, float dt);
void damage_hurt(damage *d, int amount);
int  damage_is_dead(const damage *d);

#endif
