#include "debugdraw_helpers.h"
#include "debugdraw_shapes.h"
#include "debugdraw_text.h"
#include "debugdraw_category.h"
#include "../../config.h"
#include <math.h>
void debugdraw_chunk_bounds(debugdraw *dd, const chunk *ch, ddcolor c) {
    if (!ch) return;
    if (c == 0)
        c = ch->dirty ? DDCOLOR_ORANGE : ddcolor_rgba(120, 140, 255, 160);

    float x0 = (float)(ch->cx * CHUNK_SIZE_X);
    float z0 = (float)(ch->cz * CHUNK_SIZE_Z);
    aabb a = {
        vec3_new(x0, 0.0f, z0),
        vec3_new(x0 + CHUNK_SIZE_X, (float)CHUNK_SIZE_Y, z0 + CHUNK_SIZE_Z)
    };
    debugdraw_push_category(dd, DD_CAT_WORLD);
    debugdraw_box(dd, a, c);
    debugdraw_pop_category(dd);
}

void debugdraw_block(debugdraw *dd, int bx, int by, int bz, ddcolor c) {
    // slight inset so it doesnt z-fight the block faces
    float e = 0.002f;
aabb a = {
        vec3_new((float)bx + e,     (float)by + e,     (float)bz + e),
        vec3_new((float)bx + 1 - e, (float)by + 1 - e, (float)bz + 1 - e)
    }
;
debugdraw_box(dd, a, c);
}

void debugdraw_entity(debugdraw *dd, const entity *e, ddcolor c) {
    if (!e || !e->alive) return;

    debugdraw_push_category(dd, DD_CAT_PHYSICS);

    aabb box = entity_aabb(e);
    debugdraw_box(dd, box, c);

    // facing arrow. yaw=0 looks along -z to match the camera convention.
    vec3 center = vec3_new((box.min.x + box.max.x) * 0.5f,
                           box.min.y + (box.max.y - box.min.y) * 0.55f,
                           (box.min.z + box.max.z) * 0.5f);
    vec3 fwd = vec3_new(-sinf(e->yaw), 0.0f, -cosf(e->yaw));
    vec3 tip = vec3_add(center, vec3_scale(fwd, 0.8f));
    debugdraw_arrow(dd, center, tip, 0.18f, DDCOLOR_YELLOW);

    // hp label floats above the head
    if (e->max_hp > 0) {
        vec3 above = vec3_new(center.x, box.max.y + 0.25f, center.z);
        debugdraw_labelf(dd, above, c, "%s %d/%d",
                         entity_name(e->type), e->hp, e->max_hp);
    }

    debugdraw_pop_category(dd);
}

void debugdraw_grid(debugdraw *dd, vec3 center, int half, float spacing,
                    ddcolor c) {
    if (half < 1) half = 8;
if (spacing <= 0.0f) spacing = 1.0f;
float cx = floorf(center.x / spacing) * spacing;
float cz = floorf(center.z / spacing) * spacing;
float y  = center.y;
float ext = half * spacing;
for (int i = -half;
i <= half;
vec3 tip = vec3_add(pos, vec3_scale(vel, scale));
debugdraw_arrow(dd, pos, tip, 0.15f, c);
}
