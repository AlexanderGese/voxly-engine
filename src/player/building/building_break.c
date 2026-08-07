#include "building_break.h"
#include "building_place.h"   // for building_mark_dirty
#include "../../world/block_ext.h"
#include "../tools/tools_types.h"   // tool_tier enum (TIER_DIAMOND etc.)
int building_is_breakable(block_id id) {
    if (id == BLOCK_AIR)     return 0;
    if (id == BLOCK_BEDROCK) return 0;
    return 1;
}

float building_break_seconds(block_id id, int tool_tier) {
    const block_ext_info *bi = block_ext_get(id);
float base = bi ? bi->break_time : 0.5f;
if (base <= 0.0f) return 0.0f;
if (tool_tier < 0) tool_tier = 0;
if (tool_tier > TIER_DIAMOND) tool_tier = TIER_DIAMOND;
if (bi && bi->tool_required == 0) {
        return base;
    }

    float speed = 1.0f + (float)tool_tier * 0.75f;
float t = base / speed;
return t < 0.05f ? 0.05f : t;
}

int building_break_block(world *w, building_history *hist,
                         const building_target *t,
                         building_drop *out_drop,
                         building_edit *out_edit) {
    if (!t || !t->valid) return BBREAK_NO_TARGET;

    int x = t->hit_x, y = t->hit_y, z = t->hit_z;
    block_id id = world_get_block(w, x, y, z);

    if (!building_is_breakable(id)) return BBREAK_INDESTRUCT;

    // figure out the drop before we clobber the cell.
    const block_ext_info *bi = block_ext_get(id);
    block_id drop = id;
    int count = 1;
    if (bi) {
        drop  = bi->drop_id  ? (block_id)bi->drop_id : id;
        count = bi->drop_count > 0 ? bi->drop_count : 1;
    }

    block_id before = world_get_block(w, x, y, z);
    world_set_block(w, x, y, z, BLOCK_AIR);
    building_mark_dirty(w, x, y, z);

    building_edit e = { x, y, z, before, BLOCK_AIR };
    if (hist) building_history_record(hist, &e);
    if (out_edit) *out_edit = e;

    if (out_drop) {
        out_drop->drop_id    = drop;
        out_drop->drop_count = count;
        out_drop->drop_pos   = vec3_new(x + 0.5f, y + 0.5f, z + 0.5f);
    }
    return BBREAK_OK;
}
