#ifndef ENTITY_EPHYSICS_QUERY_H
#define ENTITY_EPHYSICS_QUERY_H

#include "ephysics_types.h"
#include "../../world/world.h"

// read-only spatial queries against the block grid. the integrator already has
// the broadphase for "what might i hit while moving", but a bunch of higher-up
// code (ai grounding checks, the camera collision probe, drop-item snap) just
// wants point/box/ray answers without building a candidate set. that lives here.
//
// none of this mutates the world or any body. everything is in world meters.

// is the block at world cell (wx,wy,wz) solid enough to collide with. wraps
// block_is_solid plus the slab/plant carve-outs the broadphase uses, so probes
// agree with the actual sweep.
int ephysics_query_solid_cell(world *w, int wx, int wy, int wz);

// is world point p inside any solid block box. cheaper than a box overlap when
// you only have a point (e.g. eye position for "am i suffocating").
int ephysics_query_point_solid(world *w, vec3 p);

// does aabb `box` overlap any solid block. returns 1 on first hit. used for
// spawn validity and "can i place a body here" checks.
int ephysics_query_box_solid(world *w, aabb box);

// distance straight down from the body's feet to the first solid surface, up to
// `max`. returns `max` if nothing found within range. handy for fall prediction
// and for the ai not to walk off cliffs.
float ephysics_query_ground_dist(world *w, const ephys_body *b, float max);

// axis-aligned voxel raycast (amanatides-woo dda). marches from `origin` along
// unit `dir` up to `max_dist` meters, stops at the first solid cell. on a hit
// returns 1 and fills the cell coords, the face normal, and the hit distance.
int ephysics_query_raycast(world *w, vec3 origin, vec3 dir, float max_dist,
                           int *cx, int *cy, int *cz, vec3 *normal, float *dist);

#endif
