#ifndef UI_LOADING_H
#define UI_LOADING_H

#include "../render/text.h"

// shown during initial chunk load. simple "loading... xx%" text.

void loading_draw(text_renderer *t, const char *label, float progress,
                  int sw, int sh);

#endif
