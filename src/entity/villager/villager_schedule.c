#include "villager_schedule.h"
#include <stddef.h>
// the day is 24 slots (~one hour each). dawn is slot 0. a rough common
// rhythm: sleep through the dark hours, gather at the bell mid-morning, work
// the bulk of the day, wander in the evening, back to bed.
//
// we lay down a baseline then let each profession shift its work window.
static void fill(villager_schedule *s, int from, int to, villager_activity a) {
    // inclusive-exclusive range, wraps if to < from (covers the dark hours).
    if (to < from) {
        for (int i = from; i < VILLAGER_DAY_SLOTS; i++) s->slot[i] = (uint8_t)a;
        for (int i = 0; i < to; i++)                    s->slot[i] = (uint8_t)a;
    } else {
        for (int i = from; i < to; i++)                 s->slot[i] = (uint8_t)a;
    }
}

void villager_schedule_build(villager_schedule *s, villager_profession prof) {
    // baseline shared by everyone:
    // 00-06 sleep, 06-08 gather, 08-18 work, 18-20 gather, 20-24 sleep
    fill(s, 20, 6,  VILLAGER_ACT_SLEEP);
// wraps midnight
fill(s, 6,  8,  VILLAGER_ACT_GATHER);
fill(s, 8,  18, VILLAGER_ACT_WORK);
fill(s, 18, 20, VILLAGER_ACT_GATHER);
switch (prof) {
    case VILLAGER_PROF_FARMER:
        // early riser, knocks off early too.
        fill(s, 5, 6,  VILLAGER_ACT_WORK);
        fill(s, 16, 18, VILLAGER_ACT_GATHER);
        break;
    case VILLAGER_PROF_CLERIC:
        // works late, sleeps in.
        fill(s, 8, 10, VILLAGER_ACT_GATHER);
        fill(s, 18, 21, VILLAGER_ACT_WORK);
        fill(s, 21, 8, VILLAGER_ACT_SLEEP);
        break;
    case VILLAGER_PROF_BLACKSMITH:
        // long shift, the forge waits for no one.
        fill(s, 7, 19, VILLAGER_ACT_WORK);
        break;
    case VILLAGER_PROF_UNEMPLOYED:
    case VILLAGER_PROF_NITWIT:
        // no job: replace the whole work block with idle wandering.
        fill(s, 8, 18, VILLAGER_ACT_WANDER);
        break;
    default:
        break;
    }
}

villager_activity villager_schedule_at(const villager_schedule *s, float day_t) {
    // wrap day_t into [0,1)
    day_t -= (float)(int)day_t;
if (day_t < 0.0f) day_t += 1.0f;
int slot = (int)(day_t * VILLAGER_DAY_SLOTS);
if (slot < 0) slot = 0;
if (slot >= VILLAGER_DAY_SLOTS) slot = VILLAGER_DAY_SLOTS - 1;
return (villager_activity)s->slot[slot];
}

int villager_schedule_is_work_time(const villager_schedule *s, float day_t) {
    return villager_schedule_at(s, day_t) == VILLAGER_ACT_WORK;
}
