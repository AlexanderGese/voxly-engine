#ifndef PLAYER_BREAK_PROGRESS_H
#define PLAYER_BREAK_PROGRESS_H

// per-block break progress, driven by how long the player has been
// holding LMB on the target block. when progress hits 1.0, block breaks.

typedef struct {
    int   x, y, z;
    int   active;
    float progress;      // 0..1
    float break_time;    // seconds to break the current block
} break_progress;

void  break_progress_init(break_progress *b);
void  break_progress_begin(break_progress *b, int x, int y, int z, float break_time);
int   break_progress_tick(break_progress *b, int x, int y, int z, int holding, float dt);
int   break_progress_phase(const break_progress *b);   // 0..10 for crack overlay
void  break_progress_cancel(break_progress *b);

#endif
