#ifndef ENTITY_SPAWN_MSPAWN_QUERY_H
#define ENTITY_SPAWN_MSPAWN_QUERY_H

#include "mspawn_types.h"
#include "../../world/world.h"

// thin glue between the spawn driver and the rest of the engine's world query
// surface. the driver shouldnt know how biomes are sampled or where the
// surface is; it just asks here. keeps the worldgen/biome includes out of the
// hot driver loop and in one swappable place.

// biome at a world column. resolved through the same temperature/humidity
// sample worldgen uses so spawns agree with the terrain underfoot.
biome_id mspawn_query_biome(world *w, int wx, int wz);

// surface height (the y of the topmost surface block) at a column, for use as
// the floor scanner's y_hint. this is the generated height, which is close
// enough; the scanner refines it against the live blocks.
int mspawn_query_surface(world *w, int wx, int wz);

// is this category allowed to spawn at the surface right now? hostiles need
// night, passives need day. the driver uses this to skip whole categories
// before it bothers throwing darts.
int mspawn_query_category_open(mspawn_category cat, float day_hour);

#endif
