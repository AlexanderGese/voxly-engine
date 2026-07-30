#include "nav_debug.h"
#include "../../render/debugdraw/debugdraw_category.h"
#include "../../render/debugdraw/debugdraw_path.h"
#include "../../render/debugdraw/debugdraw_shapes.h"

// a cheap fixed palette so a region id maps to a stable, distinct-ish colour.
// 12 hues then it wraps; plenty for one patch, and the wrap rarely puts two
// touching regions on the same colour because adjacent ids flood in order.
static ddcolor region_color(int region) {
    static const ddcolor pal[12] = {
        0xff4040e0u, 0xff40e040u, 0xffe04040u, 0xff40e0e0u,
        0xffe0e040u, 0xffe040e0u, 0xff80a0ffu, 0xff80ff80u,
        0xffff80a0u, 0xff80ffa0u, 0xffa080ffu, 0xffffa040u,
    };
    if (region <= 0) return ddcolor_rgb(90, 90, 90);  // unlabelled = grey
    return pal[(region - 1) % 12];
}

void nav_debug_cells(debugdraw *dd, const nav_grid *g) {
    debugdraw_push_category(dd, DD_CAT_AI);
    for (int i = 0; i < g->count; i++) {
        vec3 c = nav_cell_world(&g->cells[i]);
        // a small flat cross sitting on the tile reads cleaner than a box for
        // a dense field. region tint, full alpha.
        debugdraw_cross(dd, c, 0.4f, region_color(g->cells[i].region));
    }
    debugdraw_pop_category(dd);
}

void nav_debug_links(debugdraw *dd, const nav_grid *g) {
    ddcolor walk = ddcolor_rgba(120, 200, 120, 160);
    ddcolor hop  = ddcolor_rgba(230, 160,  60, 200);

    debugdraw_push_category(dd, DD_CAT_AI);
    for (int i = 0; i < g->count; i++) {
        const nav_cell *c = &g->cells[i];
        vec3 from = nav_cell_world(c);
        for (int li = 0; li < c->link_count; li++) {
            int to = c->link_to[li];
            // only draw each undirected pair once: skip when the neighbour has
            // a lower index, the back-edge will draw it. one-way drops (no
            // back-edge) always have to draw here, so also draw if to > i.
            if (to < i) continue;
            vec3 dst = nav_cell_world(&g->cells[to]);
            ddcolor col = (c->link_kind[li] == NAV_LINK_WALK) ? walk : hop;
            debugdraw_line(dd, from, dst, col);
        }
    }
    debugdraw_pop_category(dd);
}

void nav_debug_region(debugdraw *dd, const nav_grid *g, int region, ddcolor c) {
    debugdraw_push_category(dd, DD_CAT_AI);
    for (int i = 0; i < g->count; i++) {
        if (g->cells[i].region != (uint16_t)region) continue;
        vec3 p = nav_cell_world(&g->cells[i]);
        // a unit box hugging the tile the cell stands on.
        aabb box = aabb_make(vec3_new(p.x - 0.45f, p.y - 0.05f, p.z - 0.45f),
                             vec3_new(p.x + 0.45f, p.y + 0.05f, p.z + 0.45f));
        debugdraw_box(dd, box, c);
    }
    debugdraw_pop_category(dd);
}

void nav_debug_path(debugdraw *dd, const nav_path *p) {
    if (p->count <= 0) return;

    // lift the path a touch so it floats above the cell markers and doesn't
    // z-fight the floor.
    vec3 lifted[NAV_PATH_MAX];
    for (int i = 0; i < p->count; i++)
        lifted[i] = vec3_add(p->pts[i], vec3_new(0, 0.25f, 0));

    ddcolor line = p->found ? ddcolor_rgb(80, 230, 110)
                            : ddcolor_rgb(230, 70, 70);
    ddcolor node = ddcolor_rgb(255, 255, 255);

    debugdraw_push_category(dd, DD_CAT_AI);
    debugdraw_path(dd, lifted, p->count, line, node, 0.0f);
    debugdraw_pop_category(dd);
}
