#include "gizmo.h"
#include "../config.h"

int gizmo_init(gizmo *g) {
    if (!wireframe_init(&g->wf)) return 0;
    if (!axes_init(&g->ax)) { wireframe_destroy(&g->wf); return 0; }
    g->visible = 0;
    return 1;
}

void gizmo_destroy(gizmo *g) {
    wireframe_destroy(&g->wf);
    axes_destroy(&g->ax);
}

void gizmo_toggle(gizmo *g) { g->visible = !g->visible; }

void gizmo_draw_chunk_bounds(gizmo *g, int cx, int cz, const camera *cam) {
    if (!g->visible) return;
    float x0 = (float)(cx * CHUNK_SIZE_X);
    float z0 = (float)(cz * CHUNK_SIZE_Z);
    aabb a = {
        { x0, 0, z0 },
        { x0 + CHUNK_SIZE_X, (float)CHUNK_SIZE_Y, z0 + CHUNK_SIZE_Z }
    };
    wireframe_draw_aabb(&g->wf, a, cam, 0.6f, 0.6f, 1.0f);
}

void gizmo_draw_aabb(gizmo *g, aabb a, const camera *cam) {
    if (!g->visible) return;
    wireframe_draw_aabb(&g->wf, a, cam, 1, 0.8f, 0);
}
