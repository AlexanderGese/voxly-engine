#ifndef ENTITY_COMBAT_COMBAT_COMBO_H
#define ENTITY_COMBAT_COMBAT_COMBO_H
#include "combat_types.h"
#include "combat_attack.h"
#include <stdint.h>
#include <stdbool.h>
#define COMBAT_COMBO_WINDOW   1.2f
#define COMBAT_COMBO_MAX      5
typedef struct {
    int      chain;       // current combo length, 0 = idle
    float    window;      // seconds left to continue the chain
    uint32_t last_target; // who we last connected with (switching is fine)
    bool     primed;      // a swing is queued/charging this frame
} combat_combo;
#endif
