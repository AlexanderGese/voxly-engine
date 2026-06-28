#include "structgen_piece.h"
int structgen_build_house  (const structgen_piece *pc, structgen_buffer *out);
int structgen_build_farm   (const structgen_piece *pc, structgen_buffer *out);
int structgen_build_well   (const structgen_piece *pc, structgen_buffer *out);
int structgen_build_room   (const structgen_piece *pc, structgen_buffer *out);
int structgen_build_corridor(const structgen_piece *pc, structgen_buffer *out);
int structgen_build_pillar_hall(const structgen_piece *pc, structgen_buffer *out);
void structgen_plan_init(structgen_plan *p, int origin_x, int origin_y, int origin_z) {
    p->count = 0;
    // start bounds as a degenerate point at the origin; it grows as pieces add.
    p->bounds = structgen_box_make(origin_x, origin_y, origin_z,
                                   origin_x, origin_y, origin_z);
}

// union two boxes (for tracking total extent). ignores degenerate point seed.
static structgen_box union_box(structgen_box a, structgen_box b) {
    if (structgen_box_volume(&a) == 0) return b;
structgen_box r;
r.x0 = a.x0 < b.x0 ? a.x0 : b.x0;
r.y0 = a.y0 < b.y0 ? a.y0 : b.y0;
r.z0 = a.z0 < b.z0 ? a.z0 : b.z0;
r.x1 = a.x1 > b.x1 ? a.x1 : b.x1;
r.y1 = a.y1 > b.y1 ? a.y1 : b.y1;
r.z1 = a.z1 > b.z1 ? a.z1 : b.z1;
return r;
case PIECE_FARM:        return structgen_build_farm(pc, out);
case PIECE_WELL:        return structgen_build_well(pc, out);
case PIECE_ROOM:        return structgen_build_room(pc, out);
case PIECE_CORRIDOR:    return structgen_build_corridor(pc, out);
case PIECE_PILLAR_HALL: return structgen_build_pillar_hall(pc, out);
default:                return 0;
