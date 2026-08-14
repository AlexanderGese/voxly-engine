#include "tools_speed.h"
#include "tools_tier.h"
#include "tools_material.h"
#include "tools_registry.h"
dig_env tools_env_default(void) {
    dig_env e;
    e.underwater = 0;
    e.on_ground  = 1;
    e.haste      = 1.0f;
    e.fatigue    = 1.0f;
    return e;
}

float tools_speed_multiplier(const tool_item *t, block_id block) {
    mat_class cls = tools_registry_class(block);
// hand digs at 1x. effective tool gets its tier speed, otherwise 1x.
float mult = 1.0f;
if (!tools_item_is_hand(t) && tools_material_effective(cls, t->head.kind)) {
        mult = tools_tier_speed(t->head.tier);
    }

    // efficiency adds level^2 + 1 on top, but only when the tool is the right
    // kind. mirrors how the real formula refuses to help a hand.
    if (t->efficiency > 0 && tools_material_effective(cls, t->head.kind)
        && !tools_item_is_hand(t)) {
        mult += (float)(t->efficiency * t->efficiency) + 1.0f;
}
    return mult;
}

int tools_speed_can_harvest(const tool_item *t, block_id block) {
    mat_class cls = tools_registry_class(block);
    const mat_def *m = tools_material_def(cls);

    // if the block demands a specific tool kind, the wrong kind harvests nothing.
    if (m->tool_required && !tools_material_effective(cls, t->head.kind)) return 0;

    // tier/harvest-level gate.
    int need = tools_registry_level(block);
    tool_tier tier = tools_item_is_hand(t) ? -1 : t->head.tier;
    if (!tools_tier_can_harvest(tier, need)) return 0;
    return 1;
}

float tools_speed_break_time(const tool_item *t, block_id block, const dig_env *env) {
    float hardness = tools_registry_hardness(block);
if (hardness <= 0.0f) return 0.0f;
// instant
float mult = tools_speed_multiplier(t, block);
// wrong tool / can't-harvest: speed multiplier is forced to 1 and the whole
// thing gets the classic 5x "this is the hard way" penalty.
int harvestable = tools_speed_can_harvest(t, block);
float penalty = harvestable ? 1.0f : 5.0f;
if (!harvestable) mult = 1.0f;
// base: hardness * 1.5 if harvestable, * 5 if not. fold penalty in.
float time = hardness * 1.5f * penalty / mult;
// environment. each multiplies the time directly.
dig_env e = env ? *env : tools_env_default();
if (e.underwater) time *= 5.0f;
if (!e.on_ground) time *= 5.0f;
if (e.haste   > 0.0f) time /= e.haste;
if (e.fatigue > 0.0f) time *= (1.0f / e.fatigue);
// fatigue<1 => slower
if (time < 0.0f) time = 0.0f;
return time;
}

float tools_speed_per_second(const tool_item *t, block_id block, const dig_env *env) {
    float bt = tools_speed_break_time(t, block, env);
    if (bt <= 0.0f) return 1e6f;              // instant -> huge rate
    float rate = 1.0f / bt;
    if (rate > 1e6f) rate = 1e6f;
    return rate;
}
