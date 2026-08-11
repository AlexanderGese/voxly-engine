#ifndef PLAYER_FISHING_BITE_H
#define PLAYER_FISHING_BITE_H
#include "fishing_types.h"
#include "fishing_rng.h"
#include "fishing_rod.h"
// the bite clock. once the bobber's floating we roll a wait, count it down,
// then open a short luring window followed by an even shorter bite window. the
// player has to react inside the bite window or the fish wanders off and we
// re-roll. all timing lives here so the session machine stays readable.
typedef struct {
    float wait;          // seconds remaining until the lure phase
    float lure_time;     // seconds of approach particles before the bite
    float bite_window;   // seconds the catch is actually grabbable
    float elapsed;       // time spent in the current phase
    int   misses;        // how many bite windows the player has whiffed
} fishing_bite;
void  fishing_bite_begin(fishing_bite *bt, fishing_rng *r,
                         const fishing_rod *rod, float quality);
int   fishing_bite_tick_wait(fishing_bite *bt, float dt);
int   fishing_bite_tick_lure(fishing_bite *bt, float dt);
int   fishing_bite_tick_window(fishing_bite *bt, float dt);
float fishing_bite_lure_progress(const fishing_bite *bt);
#endif
