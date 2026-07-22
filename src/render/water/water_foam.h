#ifndef RENDER_WATER_WATER_FOAM_H
#define RENDER_WATER_WATER_FOAM_H

#include "../../math/vec2.h"
#include "../../world/world.h"
#include "water_wave.h"

// shoreline foam. we scan the world for surface water cells that border a
// non-water block and spit out a little point cloud the surface shader (or a
// sprite pass) can draw foam at. it's regenerated when the camera moves to a
// new region, not every frame — the shoreline doesnt change that fast.

typedef struct {
    float x, z;         // world position of the foam point
    float strength;     // 0..1, how much shoreline contact here
    float phase;        // per-point animation offset so they don't pulse in sync
} water_foam_point;

typedef struct {
    water_foam_point *points;   // darray
    int   surface_y;            // the water level we scan at
    int   last_cx, last_cz;     // last region center, in blocks (snapped)
    int   region;               // half-extent in blocks we scan around the cam
} water_foam_set;

void water_foam_init(water_foam_set *s, int surface_y, int region);
void water_foam_destroy(water_foam_set *s);

// rescan around (cam_x, cam_z) if we've moved far enough. returns the number
// of foam points (0 if it decided not to rescan). the wave field modulates
// strength so foam fattens up when it's choppy.
int  water_foam_update(water_foam_set *s, world *w,
                       const water_wave_field *f, float cam_x, float cam_z);

// count of currently generated points.
int  water_foam_count(const water_foam_set *s);

#endif
