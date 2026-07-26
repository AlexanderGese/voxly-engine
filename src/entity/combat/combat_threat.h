#ifndef ENTITY_COMBAT_COMBAT_THREAT_H
#define ENTITY_COMBAT_COMBAT_THREAT_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
// threat / aggro table. one of these rides along with a mob. every time
// something damages it we bump that attacker's threat; the ai reads the top
// of the table to pick a target, and death attribution reads it to hand out
// kill credit + assists.
//
// threat decays over time so whoever last poked you doesnt stay #1 forever
// after they run off.
typedef struct {
    uint32_t attacker_id;
    float    threat;     // accumulated, decays toward 0
    float    last_dmg;   // raw damage of the most recent hit from them
    float    age;        // seconds since they last did anything
} combat_threat_entry;
// small table. a mob rarely fights more than a handful of things at once;
// when it overflows we evict the coldest entry.
#define COMBAT_THREAT_MAX 8
typedef struct {
    combat_threat_entry entries[COMBAT_THREAT_MAX];
    int count;
} combat_threat_table;
void combat_threat_init(combat_threat_table *t);
// register `amount` damage from `attacker`. id 0 (environment) is ignored —
// you cant aggro onto the floor. returns the attacker's new threat total.
float combat_threat_add(combat_threat_table *t, uint32_t attacker, float amount);
uint32_t combat_threat_top(const combat_threat_table *t);
float combat_threat_of(const combat_threat_table *t, uint32_t attacker);
bool combat_threat_drop(combat_threat_table *t, uint32_t attacker);
uint32_t combat_threat_tick(combat_threat_table *t, float dt);
int combat_threat_ranking(const combat_threat_table *t, uint32_t *out);
#endif
