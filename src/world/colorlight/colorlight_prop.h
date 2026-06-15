#ifndef WORLD_COLORLIGHT_PROP_H
#define WORLD_COLORLIGHT_PROP_H

#include "../world.h"
#include "colorlight_rgb.h"

// the flood fill itself. runs the same bfs as the scalar lighting but per
// channel and with translucent-block attenuation. everything here is world
// space and goes through colorlight_access, so floods cross chunk seams freely
// as long as the neighbor is loaded.
//
// two entry points: place light (a new emitter or a wall coming down lets light
// in) and pull light (an emitter removed or a wall going up). the remove path
// is the proper minecraft-style "unlight then relight from survivors" so we
// don't leave stale glow behind. both seed the shared queues and run to drain.

// inject light of color `col` at a voxel and flood it outward. used by the
// chunk seeder for each emitter and by the block-update path when a torch is
// placed. col is 0..255 rgb; gets narrowed to levels internally.
void colorlight_prop_place(world *w, int wx, int wy, int wz, colorlight_rgb col);

// remove whatever light currently sits at a voxel and flood the darkness,
// then relight the hole from any brighter survivors on the boundary. call when
// an emitter is destroyed or an opaque block is placed where light was.
void colorlight_prop_remove(world *w, int wx, int wy, int wz);

// flood every node already sitting in the add queue to completion. the seeder
// pushes a whole chunk of emitters then calls this once, cheaper than draining
// per emitter.
void colorlight_prop_drain_add(world *w);

#endif
