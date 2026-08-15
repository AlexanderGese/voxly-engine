#ifndef UI_HEALTHBAR_H
#define UI_HEALTHBAR_H

#include "../render/gl.h"
#include "../game/health.h"

// draws hearts, hunger drumsticks, and stamina bar above the hotbar.

void healthbar_draw(glid prog, const survival *s, int sw, int sh);

#endif
