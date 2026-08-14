#ifndef PLAYER_XP_XP_EVENT_H
#define PLAYER_XP_XP_EVENT_H

// a small ring of recent xp happenings so the hud can float "+5" popups and
// the level-up banner without us threading callbacks everywhere. write-only
// from the subsystem; the ui reads it.

#include <stdint.h>

#include "xp_config.h"
#include "../../math/vec3.h"

typedef enum {
    XP_EV_GAIN,      // absorbed some xp
    XP_EV_LEVELUP,   // crossed a level boundary
    XP_EV_REWARD,    // a reward fired
} xp_event_kind;

typedef struct {
    xp_event_kind kind;
    int    amount;    // xp for GAIN, new level for LEVELUP, reward id for REWARD
    vec3   where;     // world pos the event happened at (for floating text)
    float  age;       // seconds since posted, advanced by xp_event_tick
} xp_event;

typedef struct {
    xp_event ring[XP_EVENT_RING];
    int      head;    // next write slot
    int      count;   // number of valid entries (<= XP_EVENT_RING)
    uint32_t seq;     // monotonically increasing, lets the ui detect newness
} xp_event_log;

void  xp_event_init(xp_event_log *l);
void  xp_event_push(xp_event_log *l, xp_event_kind kind, int amount, vec3 where);

// advance ages and drop entries older than `max_age`. returns live count.
int   xp_event_tick(xp_event_log *l, float dt, float max_age);

// read the i-th most recent event (0 == newest). null if out of range.
const xp_event *xp_event_recent(const xp_event_log *l, int i);

#endif
