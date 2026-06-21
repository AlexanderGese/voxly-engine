#ifndef WORLD_LOGIC_WORLD_H
#define WORLD_LOGIC_WORLD_H
#include "logic_net.h"
#include "../world.h"
// glue between the abstract logic net and the real block world. the net is
int logic_world_scan_chunk(logic_net *net, const chunk *c);
int logic_world_scan_all(logic_net *net, world *w);
void logic_world_on_block_changed(logic_net *net, world *w, int wx, int wy, int wz);
int logic_world_lamp_lit(const logic_net *net, int wx, int wy, int wz);
uint8_t logic_world_light_at(const logic_net *net, int wx, int wy, int wz);
#endif
