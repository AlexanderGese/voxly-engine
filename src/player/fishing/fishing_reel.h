#ifndef PLAYER_FISHING_REEL_H
#define PLAYER_FISHING_REEL_H

#include "fishing_types.h"
#include "fishing_rng.h"

// the fight. once something's hooked the catch fights back with a randomly
// varying pull; the player reels in, building tension. reel too hard against a
// thrashing catch and the line snaps. let off and you make no progress. this
// is a little tension model, not the discrete grab-window — that one's in bite.

typedef struct {
    float tension;       // current line load, 0..max_tension
    float max_tension;   // copied from the rod at hook time
    float line;          // metres of line still out; reel to 0 to land it
    float fight;         // the catch's current pull, drifts via a random walk
    float fight_target;  // where the pull is drifting toward
    float stamina;       // catch tires over time, easing the fight near the end
    int   weight;        // catch heaviness 1..10, scales the pull
} fishing_reel;

// arm the fight. line is the bobber's starting distance; weight comes from the
// rolled catch's category/size. tension starts slack.
void  fishing_reel_begin(fishing_reel *rl, float max_tension,
                         float line, int weight, fishing_rng *r);

// step the catch's own pull (a damped random walk toward fight_target, with the
// target re-rolled now and then). call before applying the player's reel.
void  fishing_reel_step_fight(fishing_reel *rl, fishing_rng *r, float dt);

// apply the player reeling this frame. `reeling` is 1 while the button's held.
// builds/sheds tension and shortens the line. returns a result code.
typedef enum {
    REEL_FIGHTING = 0,   // still going
    REEL_LANDED,         // line hit zero, catch is yours
    REEL_SNAPPED         // tension maxed out, catch lost
} fishing_reel_result;

fishing_reel_result fishing_reel_apply(fishing_reel *rl, int reeling, float dt);

// 0..1 how close the line is to snapping, for a UI bar.
float fishing_reel_tension_frac(const fishing_reel *rl);

#endif
