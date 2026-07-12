#include "greedy.h"
#include "face_dir.h"
#include "face_visibility.h"
#include "ao_sampler.h"
#include "light_sample.h"
#include "../../world/block.h"

// fill the mask for one slice of one face direction. `slice` is the coord along
// the sweep axis, `face` the outward face. we look from the cell at `slice`
// across the face toward its neighbour and record a face when its visible.
static void fill_mask(const mb_ctx *c, int axis, int face, int slice,
                      mb_mask *m) {
    int ua, va;
    mb_axis_plane(axis, &ua, &va);
    int w = mb_axis_dim(ua);
    int h = mb_axis_dim(va);
    mb_mask_reset(m, w, h);

    int nx, ny, nz;
    mb_face_normal(face, &nx, &ny, &nz);

    for (int v = 0; v < h; v++) {
        for (int u = 0; u < w; u++) {
            int x, y, z;
            mb_scatter(axis, slice, u, v, &x, &y, &z);

            block_id here = c->sample(c->ctx, c->base_x + x, y, c->base_z + z);
            block_id there = c->sample(c->ctx,
                                       c->base_x + x + nx, y + ny,
                                       c->base_z + z + nz);

            if (!mb_face_visible(here, there)) continue;

            mb_cell *cell = mb_mask_at(m, u, v);
            cell->present = 1;
            cell->block   = here;
            cell->tile    = (uint8_t)block_face_tile(here, face);

            int ao[4], lt[4];
            mb_ao_quad(c, x, y, z, face, ao);
            mb_light_quad(c, x, y, z, face, lt);
            for (int i = 0; i < 4; i++) {
                cell->ao[i]    = (uint8_t)ao[i];
                cell->light[i] = (uint8_t)lt[i];
            }
        }
    }
}

// emit one merged quad from a mask rectangle at (u,v) of size (qw,qh).
static void emit_rect(const mb_ctx *c, int axis, int face, int slice,
                      int u, int v, int qw, int qh, const mb_cell *cell,
                      mb_emit_fn emit, void *user) {
    int x, y, z;
    mb_scatter(axis, slice, u, v, &x, &y, &z);

    // positive faces sit on the far side of the cell along the normal; the
    // greedy origin is the cell min corner, so push positive faces out by one.
    int nx, ny, nz;
    mb_face_normal(face, &nx, &ny, &nz);
    if (nx > 0) x += 1;
    if (ny > 0) y += 1;
    if (nz > 0) z += 1;

    mb_quad q;
    q.face  = (uint8_t)face;
    q.tile  = cell->tile;
    q.light = cell->light[0];     // representative; per-corner ao still applied
    q.x = (float)x; q.y = (float)y; q.z = (float)z;

    int ua, va;
    mb_axis_plane(axis, &ua, &va);
    (void)ua; (void)va;
    q.du = (float)qw;
    q.dv = (float)qh;

    // the rect shares one shading profile (we only merged equal cells), so the
    // corner ao of the origin cell is valid for the whole quad.
    for (int i = 0; i < 4; i++) q.ao[i] = cell->ao[i];

    emit(user, &q, cell->block, c->base_x, c->base_z);
}

// merge the filled mask into rectangles. greedy: grow width along u while cells
// match, then grow height along v while the whole row matches, then stamp the
// rect consumed so we dont revisit it.
static void merge_mask(const mb_ctx *c, int axis, int face, int slice,
                       mb_mask *m, mb_emit_fn emit, void *user) {
    int merge = c->merge;

    for (int v = 0; v < m->h; v++) {
        for (int u = 0; u < m->w; ) {
            mb_cell *start = mb_mask_at(m, u, v);
            if (!start->present) { u++; continue; }

            // width run
            int qw = 1;
            if (merge) {
                while (u + qw < m->w &&
                       mb_cell_eq(start, mb_mask_at(m, u + qw, v)))
                    qw++;
            }

            // height run: every cell in the candidate row must match.
            int qh = 1;
            if (merge) {
                int grow = 1;
                while (v + qh < m->h && grow) {
                    for (int k = 0; k < qw; k++) {
                        if (!mb_cell_eq(start, mb_mask_at(m, u + k, v + qh))) {
                            grow = 0;
                            break;
                        }
                    }
                    if (grow) qh++;
                }
            }

            emit_rect(c, axis, face, slice, u, v, qw, qh, start, emit, user);

            // consume the rect.
            for (int dv = 0; dv < qh; dv++)
                for (int du = 0; du < qw; du++)
                    mb_mask_at(m, u + du, v + dv)->present = 0;

            u += qw;
        }
    }
}

void mb_greedy_axis(const mb_ctx *c, int axis, mb_mask *m,
                    mb_emit_fn emit, void *user) {
    int dim = mb_axis_dim(axis);
    int pos = mb_axis_pos_face(axis);
    int neg = mb_axis_neg_face(axis);

    for (int slice = 0; slice < dim; slice++) {
        fill_mask(c, axis, pos, slice, m);
        merge_mask(c, axis, pos, slice, m, emit, user);

        fill_mask(c, axis, neg, slice, m);
        merge_mask(c, axis, neg, slice, m, emit, user);
    }
}
