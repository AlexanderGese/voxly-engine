#include "mineshaft_build.h"
#include "mineshaft_box.h"
#include "mineshaft_support.h"
#include "mineshaft_cobweb.h"
#include "mineshaft_ore.h"
#include "mineshaft_rails.h"
#include "mineshaft_light.h"
#include "../block_ext.h"
#define SHAFT_DROP   8
#define WEB_DENSITY  0.12f
int mineshaft_build_grid_dim(const mineshaft_config *cfg) {
    // pick the largest square grid whose cell count stays under max_pieces and
    // the static cap. integer sqrt by trial - grids are tiny so it's free.
    int dim = 1;
    while ((dim + 1) * (dim + 1) <= cfg->max_pieces && dim + 1 <= MINESHAFT_GRID_MAX)
        dim++;
    return dim;
}

mineshaft_box mineshaft_build_cell_box(const mineshaft_site *site,
                                       const mineshaft_config *cfg,
                                       int gdim, int cx, int cz) {
    // grid is centered on the anchor. cell (0,0) is the top-left corner.
    int pitch  = cfg->corridor_len;
int half_w = (gdim * pitch) / 2;
int half_d = (gdim * pitch) / 2;
int wx = site->anchor_x - half_w + cx * pitch;
int wz = site->anchor_z - half_d + cz * pitch;
return mineshaft_box_at(wx, site->floor_y, wz,
                            pitch, MINESHAFT_CORRIDOR_H + 2, pitch);
}

// carve the walkable passage: a stone-lined box hollowed to air. fill comes from
// cfg->mat_fill so the walls match the rest of the shaft's backing.
static int carve_passage(mineshaft_buffer *b, const mineshaft_config *cfg,
                         mineshaft_box cell, int floor_y, int ceil_y) {
    mineshaft_box box = mineshaft_box_make(cell.x0, floor_y, cell.z0,
                                           cell.x1, ceil_y + 1, cell.z1);
    return mineshaft_buffer_carve_room(b, box, cfg->mat_fill, BLOCK_AIR);
}

// knock a doorway through the wall toward each linked neighbour so adjacent
// cells actually connect. width 1, full corridor height.
static int open_doorways(mineshaft_buffer *b, const mineshaft_cell *c,
                         mineshaft_box cell, int floor_y, int ceil_y) {
    int n = 0;
int mx = (cell.x0 + cell.x1) / 2;
int mz = (cell.z0 + cell.z1) / 2;
for (int d = 0;
d < 4;
d++) {
        if (!(c->links & (1 << d))) continue;
        int dx, dz;
        mineshaft_dir_step((mineshaft_dir)d, &dx, &dz);
        int wx = (dx > 0) ? cell.x1 - 1 : (dx < 0) ? cell.x0 : mx;
        int wz = (dz > 0) ? cell.z1 - 1 : (dz < 0) ? cell.z0 : mz;
        for (int y = floor_y + 1; y <= ceil_y; y++)
            n += mineshaft_buffer_add(b, wx, y, wz, BLOCK_AIR);
    }
    return n;
}

int mineshaft_build_cell(mineshaft_buffer *b, mineshaft_grid *g,
                         const mineshaft_site *site, const mineshaft_config *cfg,
                         int cx, int cz, mineshaft_rng *rng) {
    mineshaft_cell *c = mineshaft_grid_at(g, cx, cz);
    if (c->kind == MS_CELL_EMPTY) return 0;

    int gdim = g->w;   // grid is square; w == d
    mineshaft_box cell = mineshaft_build_cell_box(site, cfg, gdim, cx, cz);
    int floor_y = site->floor_y;
    int ceil_y  = floor_y + MINESHAFT_CORRIDOR_H;
    uint32_t cseed = mineshaft_seed_mix(site->seed,
                                        (uint32_t)((cz << 8) ^ cx ^ 0x4d59u));
    int n = 0;

    switch (c->kind) {
        case MS_CELL_ROOM:
            // ore room handles its own (taller) carve.
            n += mineshaft_ore_room(b, cfg, cell, floor_y, ceil_y + 1, rng, cseed);
            n += open_doorways(b, c, cell, floor_y, ceil_y);
            n += mineshaft_cobweb_fill(b, cfg, cell, floor_y, ceil_y + 1,
                                       WEB_DENSITY * 1.5f, cseed);
            break;

        case MS_CELL_JUNCTION:
            n += carve_passage(b, cfg, cell, floor_y, ceil_y);
            n += open_doorways(b, c, cell, floor_y, ceil_y);
            n += mineshaft_support_junction(b, cfg, cell, floor_y, ceil_y, rng);
            n += mineshaft_cobweb_fill(b, cfg, cell, floor_y, ceil_y,
                                       WEB_DENSITY, cseed);
            break;

        case MS_CELL_SHAFT: {
            n += carve_passage(b, cfg, cell, floor_y, ceil_y);
            n += open_doorways(b, c, cell, floor_y, ceil_y);
            // punch a vertical drop with a ladder column against one wall.
            int lx = cell.x0 + 1, lz = cell.z0 + 1;
            int bottom = floor_y - SHAFT_DROP;
            if (bottom < cfg->depth_min) bottom = cfg->depth_min;
            for (int y = bottom; y <= ceil_y; y++) {
                n += mineshaft_buffer_add(b, lx, y, lz, BLOCK_AIR);
                n += mineshaft_buffer_add(b, lx, y, lz - 1, BLOCK_LADDER);
            }
            break;
        }

        case MS_CELL_DEADEND:
            n += carve_passage(b, cfg, cell, floor_y, ceil_y);
            n += open_doorways(b, c, cell, floor_y, ceil_y);
            if (c->flags & MS_FLAG_COLLAPSED) {
                // partial cave-in: a pile of gravel fills part of the stub.
                mineshaft_box pile = mineshaft_box_inset(cell, 1, 0);
                for (int z = pile.z0; z < pile.z1; z++)
                    for (int x = pile.x0; x < pile.x1; x++)
                        if (mineshaft_hash2(x, z, cseed) & 1)
                            n += mineshaft_buffer_post(b, x, floor_y + 1,
                                                       floor_y + 1 +
                                                       mineshaft_rng_range(rng, 1, 2),
                                                       z, BLOCK_GRAVEL);
            }
            // dead ends are the webbiest places in the shaft.
            n += mineshaft_cobweb_fill(b, cfg, cell, floor_y, ceil_y,
                                       WEB_DENSITY * 2.0f, cseed);
            break;

        case MS_CELL_CORRIDOR:
        default:
            n += carve_passage(b, cfg, cell, floor_y, ceil_y);
            n += open_doorways(b, c, cell, floor_y, ceil_y);
            n += mineshaft_support_cell(b, cfg, g, cx, cz, cell,
                                        floor_y, ceil_y, rng);
            // rails down straight runs, worn thin toward the middle of the maze.
            {
                float wear = 0.25f + (float)c->depth * 0.04f;
                if (wear > 0.8f) wear = 0.8f;
                n += mineshaft_rails_cell(b, cfg, g, cx, cz, cell,
                                          floor_y, wear, rng);
            }
            n += mineshaft_cobweb_fill(b, cfg, cell, floor_y, ceil_y,
                                       WEB_DENSITY, cseed);
            break;
    }

    // sparse lighting goes on every carved cell, biased by kind so junctions and
    // landings stay lit while plain corridors fade to dark. last pass so torches
    // land in air, not inside a beam.
    {
        float surv = mineshaft_light_survival(g, cx, cz);
        n += mineshaft_light_cell(b, cfg, cell, floor_y, ceil_y, surv, cseed);
    }
    return n;
}

int mineshaft_build_grid(mineshaft_buffer *b, mineshaft_grid *g,
                         const mineshaft_site *site, const mineshaft_config *cfg,
                         mineshaft_rng *rng) {
    int n;
for (int z;
z < g->d;
}
