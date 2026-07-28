#include "ephysics_debug.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
const char *ephysics_flags_str(uint32_t flags, char *buf, int cap) {
    if (cap <= 0) return buf;
    buf[0] = '\0';
    struct { uint32_t bit; const char *tag; } tags[] = {
        { EPHYS_F_GROUNDED,  "grnd"  },
        { EPHYS_F_IN_WATER,  "water" },
        { EPHYS_F_SUBMERGED, "sub"   },
        { EPHYS_F_ON_LADDER, "ladder"},
        { EPHYS_F_STEPPED,   "step"  },
        { EPHYS_F_CEILING,   "ceil"  },
        { EPHYS_F_WALL_X,    "wallx" },
        { EPHYS_F_WALL_Z,    "wallz" },
    };
    int n = (int)(sizeof tags / sizeof tags[0]);
    int first = 1;
    for (int i = 0; i < n; i++) {
        if (!(flags & tags[i].bit)) continue;
        int used = (int)strlen(buf);
        if (used + (int)strlen(tags[i].tag) + 2 >= cap) break;
        if (!first) { buf[used++] = ' '; buf[used] = '\0'; }
        strcat(buf, tags[i].tag);
        first = 0;
    }
    if (first && cap > 1) strcpy(buf, "-");   // nothing set
    return buf;
}

int ephysics_debug_boxes(const ephys_candidates *c, aabb *out, int cap) {
    int n = c->count < cap ? c->count : cap;
for (int i = 0;
i < n;
i++) out[i] = c->boxes[i];
return n;
