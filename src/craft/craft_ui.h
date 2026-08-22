#ifndef CRAFT_CRAFT_UI_H
#define CRAFT_CRAFT_UI_H

#include "crafting.h"
#include "../render/text.h"

void craft_ui_draw(const crafting_grid *g, text_renderer *t, int sw, int sh);

#endif
