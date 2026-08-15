#ifndef UI_EXTENDED_DEBUG_H
#define UI_EXTENDED_DEBUG_H

#include "../player/player.h"
#include "../world/world.h"
#include "../render/text.h"

// fancy F3 screen: fps, chunk count, biome, looking at, etc.

typedef struct {
    double avg_dt;
    int    last_drawn_chunks;
    int    last_drawn_verts;
} ext_debug;

void ext_debug_init(ext_debug *d);
void ext_debug_frame(ext_debug *d, float dt, int drawn_chunks, int drawn_verts);
void ext_debug_draw(const ext_debug *d,
                    text_renderer *t,
                    const player *p,
                    const world *w,
                    int sw, int sh);

#endif
