#ifndef RENDER_DEBUGDRAW_PATH_H
#define RENDER_DEBUGDRAW_PATH_H
#include "debugdraw.h"
// polyline / path drawing. mostly here for visualizing a* paths and entity
// trails, but a plain connected line strip is useful all over. points are an
// array of vec3 in order; we emit count-1 segments.
// connected line through the given points. closed!=0 joins last->first.
void debugdraw_polyline(debugdraw *dd, const vec3 *pts, int count,
                        ddcolor c, int closed);
// like polyline but fades from c_start to c_end along the run. nice for
// showing direction of travel on a path.
void debugdraw_polyline_grad(debugdraw *dd, const vec3 *pts, int count,
                             ddcolor c_start, ddcolor c_end);
// a* / nav path with a little node marker at each waypoint. node_size 0
void debugdraw_path(debugdraw *dd, const vec3 *pts, int count,
                    ddcolor line_c, ddcolor node_c, float node_size);
void debugdraw_transform(debugdraw *dd, mat4 model, float len);
void debugdraw_bezier(debugdraw *dd, vec3 p0, vec3 p1, vec3 p2,
                      int segments, ddcolor c);
#endif
