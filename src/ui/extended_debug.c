#include "extended_debug.h"
#include "../world/biome.h"
#include "../world/block.h"
#include "../config.h"

#include <stdio.h>

void ext_debug_init(ext_debug *d) {
    d->avg_dt = 0.016;
    d->last_drawn_chunks = 0;
    d->last_drawn_verts = 0;
}

void ext_debug_frame(ext_debug *d, float dt, int drawn_chunks, int drawn_verts) {
    d->avg_dt = d->avg_dt * 0.92 + dt * 0.08;
    d->last_drawn_chunks = drawn_chunks;
    d->last_drawn_verts  = drawn_verts;
}

void ext_debug_draw(const ext_debug *d,
                    text_renderer *t,
                    const player *p,
                    const world *w,
                    int sw, int sh) {
    double fps = d->avg_dt > 0 ? 1.0 / d->avg_dt : 0;

    float bt, bh;
    biome_sample((int)p->pos.x, (int)p->pos.z, w->seed, &bt, &bh);
    biome_id  bid = biome_pick(bt, bh);
    const biome_info *bi = biome_get(bid);

    char buf[512];
    snprintf(buf, sizeof buf,
        "voxel [debug]\n"
        "fps   %.1f  (%.2fms)\n"
        "pos   %.1f %.1f %.1f\n"
        "rot   yaw=%.2f pitch=%.2f\n"
        "grnd  %s   fly %s\n"
        "chunk %d loaded  %d drawn  verts %d\n"
        "biome %s  t=%.2f h=%.2f\n"
        "seed  %u\n",
        fps, d->avg_dt * 1000.0,
        p->pos.x, p->pos.y, p->pos.z,
        p->yaw, p->pitch,
        p->on_ground ? "yes" : "no", p->flying ? "on" : "off",
        w->count, d->last_drawn_chunks, d->last_drawn_verts,
        bi->name, bt, bh,
        w->seed);
    text_draw(t, buf, 8, 8, 1, 1, 1, sw, sh);
}
