#ifndef WORLD_LIGHTPROP_SAMPLE_H
#define WORLD_LIGHTPROP_SAMPLE_H
#include "lightprop_types.h"
// the read side for the mesher. flat per-face light looks awful at block edges
// (hard banding), so we average the light of the four cells touching each vertex
// of a face. this is the "smooth lighting" half of the classic AO+smooth combo;
// the render/ side does the actual AO darkening, we just hand it nicely blended
// light values.
// combined light at a cell as a 0..1 float, taking max(block, sky*daylight).
// `daylight` is the current day/night sky multiplier (0..1) so caves stay dark
// at noon and the surface dims at night. opaque cells return 0.
float lp_sample_cell(world *w, int wx, int wy, int wz, float daylight);
// sample the light for one vertex of one face. `face` is 0..5 (matching
float lp_sample_vertex(world *w, int wx, int wy, int wz,
                       int face, int corner, float daylight);
void  lp_sample_face_raw(world *w, int wx, int wy, int wz, int face,
                        uint8_t *out_block, uint8_t *out_sky);
#endif
