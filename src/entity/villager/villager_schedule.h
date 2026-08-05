#ifndef ENTITY_VILLAGER_SCHEDULE_H
#define ENTITY_VILLAGER_SCHEDULE_H

#include "villager_types.h"

// the daily schedule maps a normalized day clock (t in [0,1), 0 = dawn) onto
// an activity. different professions keep slightly different hours — the
// farmer is up at first light, the cleric works late. unemployed villagers
// have no WORK slot at all and just wander or gather.
//
// the table is built once per profession into VILLAGER_DAY_SLOTS buckets so
// lookups are a single index, not a chain of if/else over float ranges.

typedef struct {
    uint8_t slot[VILLAGER_DAY_SLOTS];   // villager_activity per hour-slot
} villager_schedule;

// fill `s` with the schedule for a profession.
void villager_schedule_build(villager_schedule *s, villager_profession prof);

// what should this villager be doing at day-fraction `day_t` in [0,1)?
villager_activity villager_schedule_at(const villager_schedule *s, float day_t);

// convenience: is it currently work hours for this schedule?
int villager_schedule_is_work_time(const villager_schedule *s, float day_t);

#endif
