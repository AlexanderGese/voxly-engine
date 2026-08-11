#include "fishing_reel.h"
#define REEL_BASE_SPEED   3.0f
#define REEL_TENSION_GAIN 14.0f
#define REEL_TENSION_BLEED 22.0f
#define FIGHT_MIN   0.2f
#define FIGHT_MAX   1.0f
otherwise drift toward the current
    // one. roughly a re-roll every ~1.2s on average.
    if (fishing_rng_chance(r, dt / 1.2f))
        rl->fight_target = fishing_rng_frange(r, FIGHT_MIN, FIGHT_MAX);
float k = 3.0f * dt;
if (k > 1.0f) k = 1.0f;
rl->fight += (rl->fight_target - rl->fight) * k;
rl->stamina -= dt * 0.05f;
if (rl->stamina < 0.25f) rl->stamina = 0.25f;
