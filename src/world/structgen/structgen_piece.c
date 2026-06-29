#include "structgen_piece.h"

// forward decls of the per-type builders. each lives in its own file so the
// dispatch table here stays the only place that knows them all.
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
}

int structgen_plan_try_add(structgen_plan *p, structgen_piece_type type,
                           structgen_box footprint, structgen_dir facing,
                           uint32_t seed) {
    if (p->count >= (int)(sizeof p->pieces / sizeof p->pieces[0])) return 0;

    // reject if it would overlap an already-placed piece. corridors are the
    // exception: they're meant to butt up against rooms, so we let them touch
    // but not deeply overlap (shrink the test box by 1 on xz).
    structgen_box test = footprint;
    if (type == PIECE_CORRIDOR)
        test = structgen_box_inset(test, 1, 0);

    for (int i = 0; i < p->count; i++) {
        if (structgen_box_overlaps(&test, &p->pieces[i].footprint))
            return 0;
    }

    structgen_piece *pc = &p->pieces[p->count++];
    pc->type      = type;
    pc->footprint = footprint;
    pc->facing    = facing;
    pc->seed      = seed;

    p->bounds = union_box(p->bounds, footprint);
    return 1;
}

int structgen_piece_build(const structgen_piece *pc, structgen_buffer *out) {
    switch (pc->type) {
        case PIECE_HOUSE:       return structgen_build_house(pc, out);
        case PIECE_FARM:        return structgen_build_farm(pc, out);
        case PIECE_WELL:        return structgen_build_well(pc, out);
        case PIECE_ROOM:        return structgen_build_room(pc, out);
        case PIECE_CORRIDOR:    return structgen_build_corridor(pc, out);
        case PIECE_PILLAR_HALL: return structgen_build_pillar_hall(pc, out);
        default:                return 0;
    }
}

int structgen_plan_build(const structgen_plan *p, structgen_buffer *out) {
    int n = 0;
    for (int i = 0; i < p->count; i++)
        n += structgen_piece_build(&p->pieces[i], out);
    return n;
}
