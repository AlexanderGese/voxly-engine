#include "profiler_ui.h"
#include "../util/profiler.h"

#include <stdio.h>

void profiler_ui_draw(text_renderer *t, int sw, int sh) {
    int n = prof_count();
    if (n == 0) return;
    int y = 8;
    int x = sw - 200;
    text_draw(t, "profiler", x, y, 1, 1, 0.4f, sw, sh);
    y += 14;
    for (int i = 0; i < n; i++) {
        char line[96];
        snprintf(line, sizeof line, "%-12s %6.2f ms", prof_name(i), prof_avg_ms(i));
        text_draw(t, line, x, y, 1, 1, 1, sw, sh);
        y += 10;
    }
}
