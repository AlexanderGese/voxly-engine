#ifndef ENTITY_VILLAGER_BREED_H
#define ENTITY_VILLAGER_BREED_H

#include "villager.h"
#include "villager_manager.h"
#include "villager_poi.h"

// reproduction. villagers become "willing" when they're well-fed and there's
// a spare bed to put a baby in. two willing adults near each other and near
// the village center will breed, spawning a baby and consuming willingness.
//
// food/willingness is abstracted: trading and working top up a food counter
// (proxy for the farmer handing out bread), idleness slowly drains it.

// willingness thresholds, in arbitrary "food" points.
#define VILLAGER_BREED_FOOD_NEED   12
#define VILLAGER_BREED_COOLDOWN    600.0f   // seconds between breedings
// how close two adults must be to pair up.
#define VILLAGER_BREED_RANGE        4.0f

// is this villager currently willing to breed?
int villager_breed_is_willing(const villager *v, float food, float cooldown);

// feed a villager: bump its food toward the willingness threshold. clamped.
// returns the new food value.
float villager_breed_feed(float food, float amount);

// per-village breeding pass. scans for willing pairs and spawns babies when
// a free bed exists. mutates per-villager food/cooldown via the parallel
// arrays the caller supplies (one float each per villager, indexed by the
// manager order). returns the number of babies born this pass.
int villager_breed_step(villager_manager *m, float *food, float *cooldown,
                        float dt);

#endif
