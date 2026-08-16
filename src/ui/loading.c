#include "loading.h"

#include <stdio.h>

void loading_draw(text_renderer *t, const char *label, float progress,
                  int sw, int sh) {
    char buf[128];
    int pct = (int)(progress * 100.0f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    snprintf(buf, sizeof buf, "%s %d%%", label, pct);

    int cx = sw / 2;
    int cy = sh / 2;
    text_draw(t, "voxel", cx - 20, cy - 30, 1, 1, 1, sw, sh);
    text_draw(t, buf, cx - 40, cy, 1, 1, 1, sw, sh);
}
