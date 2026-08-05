#include "damage.h"

void damage_init(damage *d) {
    d->hp = PLAYER_MAX_HP;
    d->prev_vel_y = 0;
    d->hit_flash = 0;
}

void damage_update(damage *d, const player *p, float dt) {
    // fall damage: if just hit ground while moving down fast
    if (p->on_ground && d->prev_vel_y < -12.0f) {
        int dmg = (int)((-d->prev_vel_y - 12.0f) * 0.5f);
        if (dmg > 0) damage_hurt(d, dmg);
    }
    d->prev_vel_y = p->vel.y;
    if (d->hit_flash > 0) d->hit_flash -= dt;
    if (d->hit_flash < 0) d->hit_flash = 0;
}

void damage_hurt(damage *d, int amount) {
    d->hp -= amount;
    if (d->hp < 0) d->hp = 0;
    d->hit_flash = 0.3f;
}

int damage_is_dead(const damage *d) {
    return d->hp <= 0;
}
