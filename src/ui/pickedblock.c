#include "pickedblock.h"
#include "../math/aabb.h"

void pickedblock_draw(wireframe *wf, ray_hit h, const camera *cam) {
    if (!h.hit) return;
    aabb a = {
        {(float)h.x - 0.005f, (float)h.y - 0.005f, (float)h.z - 0.005f},
        {(float)h.x + 1.005f, (float)h.y + 1.005f, (float)h.z + 1.005f}
    };
    wireframe_draw_aabb(wf, a, cam, 0, 0, 0);
}
