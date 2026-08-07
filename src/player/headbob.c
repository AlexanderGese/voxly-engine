#include "headbob.h"

#include <math.h>

void headbob_init(headbob *hb) {
    hb->phase = 0;
    hb->amp = 0;
}

void headbob_update(headbob *hb, float speed, float dt) {
    // if walking, accumulate phase; else decay amp
    if (speed > 0.5f) {
        hb->phase += speed * dt * 6.0f;
        if (hb->amp < 0.08f) hb->amp += dt * 0.3f;
    } else {
        if (hb->amp > 0) hb->amp -= dt * 0.3f;
        if (hb->amp < 0) hb->amp = 0;
    }
}

float headbob_offset(const headbob *hb) {
    return sinf(hb->phase) * hb->amp;
}
