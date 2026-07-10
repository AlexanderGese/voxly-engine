#ifndef RENDER_LODTERRAIN_LT_MESH_H
#define RENDER_LODTERRAIN_LT_MESH_H

#include "lt_types.h"

// cpu-side mesh buffer plumbing. the surface and skirt passes both append into
// one of these via lt_mesh_quad. verts/indices are darrays so we never have to
// guess a final size up front.

// zero a fresh mesh and reserve the configured starting capacity. cheap to call
// per chunk; reset reuses the allocation instead.
void lt_mesh_init(lt_mesh *m);

// clear length but keep the backing storage, ready for a rebuild. quad/skirt
// counters reset too.
void lt_mesh_reset(lt_mesh *m);

// free both darrays and zero the struct.
void lt_mesh_free(lt_mesh *m);

// push one quad as two triangles. corners must be given in ccw order when
// viewed from outside the surface (v0->v1->v2->v3). indices are generated as
// (0,1,2) (0,2,3). `is_skirt` only bumps the skirt counter for stats.
void lt_mesh_quad(lt_mesh *m,
                  lt_vertex v0, lt_vertex v1, lt_vertex v2, lt_vertex v3,
                  int is_skirt);

// current triangle count, for the cache budget + F3 overlay.
int  lt_mesh_tris(const lt_mesh *m);

#endif
