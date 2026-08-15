#ifndef UI_HUD_H
#define UI_HUD_H

#include "../player/inventory.h"
#include "../render/gl.h"

void hud_init(void);
void hud_draw(glid prog, const inventory *inv, int screen_w, int screen_h);

#endif
