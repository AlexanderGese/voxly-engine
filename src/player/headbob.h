#ifndef PLAYER_HEADBOB_H
#define PLAYER_HEADBOB_H

// tiny head bob offset for walking. fed into camera.pos.y between physics and draw.

typedef struct {
    float phase;
    float amp;
} headbob;

void  headbob_init(headbob *hb);
void  headbob_update(headbob *hb, float speed, float dt);
float headbob_offset(const headbob *hb);

#endif
