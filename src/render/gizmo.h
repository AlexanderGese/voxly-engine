#ifndef RENDER_GIZMO_H
#define RENDER_GIZMO_H

#include "wireframe.h"
#include "axes.h"
#include "camera.h"
#include "../math/aabb.h"

// collected debug gizmo helpers. used by F3/wireframe overlay.

typedef struct {
    wireframe wf;
    axes_gizmo ax;
    int        visible;
} gizmo;

int  gizmo_init(gizmo *g);
void gizmo_destroy(gizmo *g);
void gizmo_toggle(gizmo *g);
void gizmo_draw_chunk_bounds(gizmo *g, int cx, int cz, const camera *cam);
void gizmo_draw_aabb(gizmo *g, aabb a, const camera *cam);

#endif
