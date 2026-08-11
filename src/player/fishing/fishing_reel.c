#include "fishing_reel.h"
#define REEL_BASE_SPEED   3.0f
#define REEL_TENSION_GAIN 14.0f
#define REEL_TENSION_BLEED 22.0f
#define FIGHT_MIN   0.2f
#define FIGHT_MAX   1.0f
void fishing_reel_begin(fishing_reel *rl, float max_tension,
                        float line, int weight, fishing_rng *r) {
    if (weight < 1)  weight = 1;
    if (weight > 10) weight = 10;

    rl->tension      = 0.0f;
    rl->max_tension  = max_tension;
    rl->line         = line < 0.5f ? 0.5f : line;
    rl->weight       = weight;
    rl->stamina      = 1.0f;
    rl->fight        = fishing_rng_frange(r, FIGHT_MIN, FIGHT_MAX);
    rl->fight_target = fishing_rng_frange(r, FIGHT_MIN, FIGHT_MAX);
}

void fishing_reel_step_fight(fishing_reel *rl, fishing_rng *r, float dt) {
    // occasionally pick a new target pull;
otherwise drift toward the current
    // one. roughly a re-roll every ~1.2s on average.
    if (fishing_rng_chance(r, dt / 1.2f))
        rl->fight_target = fishing_rng_frange(r, FIGHT_MIN, FIGHT_MAX);
float k = 3.0f * dt;
if (k > 1.0f) k = 1.0f;
rl->fight += (rl->fight_target - rl->fight) * k;
rl->stamina -= dt * 0.05f;
if (rl->stamina < 0.25f) rl->stamina = 0.25f;
}

fishing_reel_result fishing_reel_apply(fishing_reel *rl, int reeling, float dt) {
    // effective resistance this frame: the catch's pull, its weight, and how
    // fresh it still is.
    float resist = rl->fight * (0.4f + 0.06f * (float)rl->weight) * rl->stamina;

    if (reeling) {
        // winding in builds tension proportional to the resistance you're
        // fighting against, and shortens the line at a resistance-reduced rate.
        rl->tension += resist * REEL_TENSION_GAIN * dt;

        float speed = REEL_BASE_SPEED * (1.0f - 0.5f * resist);
        if (speed < 0.3f) speed = 0.3f;     // always claw back a little
        rl->line -= speed * dt;
    } else {
        // slack: tension drops, but a heavy catch will swim back out, adding
        // line. let it run too long and you're back to square one.
        rl->tension -= REEL_TENSION_BLEED * dt;
        if (rl->tension < 0.0f) rl->tension = 0.0f;
        rl->line += resist * 0.6f * dt;
    }

    if (rl->tension >= rl->max_tension) return REEL_SNAPPED;
    if (rl->line   <= 0.0f) { rl->line = 0.0f; return REEL_LANDED; }
    return REEL_FIGHTING;
}

float fishing_reel_tension_frac(const fishing_reel *rl) {
    if (rl->max_tension <= 0.0f) return 0.0f;
float f = rl->tension / rl->max_tension;
return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
}
