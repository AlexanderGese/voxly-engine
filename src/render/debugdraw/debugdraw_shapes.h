#ifndef RENDER_DEBUGDRAW_SHAPES_H
#define RENDER_DEBUGDRAW_SHAPES_H

#include "debugdraw.h"
#include "../../math/frustum.h"

// higher level shapes built on top of the line/point primitives. all of
// these just emit segments into the active debugdraw buckets, so they
// respect the current depth state.

// wireframe sphere drawn as three orthogonal great circles. `segments` is
// the smoothness per circle (8..32 sensible). pass 0 for a default.
void debugdraw_sphere(debugdraw *dd, vec3 center, float radius,
                      int segments, ddcolor c);

// a single circle on an arbitrary plane defined by its normal.
void debugdraw_circle(debugdraw *dd, vec3 center, vec3 normal,
                      float radius, int segments, ddcolor c);

// 3-axis cross / star at a point. size is the full extent.
void debugdraw_cross(debugdraw *dd, vec3 center, float size, ddcolor c);

// arrow from->to with a little arrowhead. head_size is the cone length.
void debugdraw_arrow(debugdraw *dd, vec3 from, vec3 to,
                     float head_size, ddcolor c);

// oriented box from a model matrix applied to a unit cube centered at origin
// (so it can be rotated, unlike debugdraw_box which is axis aligned).
void debugdraw_obb(debugdraw *dd, mat4 model, ddcolor c);

// draw a camera/view frustum from its 6 planes by reconstructing the 8
// corners. handy for visualizing culling.
void debugdraw_frustum(debugdraw *dd, const frustum *f, ddcolor c);

// cone from apex along dir, with the given base radius and height.
void debugdraw_cone(debugdraw *dd, vec3 apex, vec3 dir,
                    float radius, float height, int segments, ddcolor c);

// vertical capsule (two hemispheres-ish via rings + a connecting cylinder).
// approximated, good enough to eyeball an entity collider.
void debugdraw_capsule(debugdraw *dd, vec3 base, float radius,
                       float height, int segments, ddcolor c);

#endif
