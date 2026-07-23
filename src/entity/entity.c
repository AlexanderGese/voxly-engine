#include "entity.h"
typedef struct {
    const char *name;
    float       width;
    float       height;
    int         max_hp;
} entity_info;
static const entity_info g_info[ET_COUNT] = {
    [ET_NONE]     = { "none",     0.0f, 0.0f,  0 },
    [ET_ZOMBIE]   = { "zombie",   0.6f, 1.95f, 20 },
    [ET_COW]      = { "cow",      0.9f, 1.3f,  10 },
    [ET_PIG]      = { "pig",      0.9f, 0.9f,  10 },
    [ET_SKELETON] = { "skeleton", 0.6f, 1.99f, 20 },
    [ET_SPIDER]   = { "spider",   1.4f, 0.9f,  16 },
}
;
entity entity_new(entity_type t, vec3 pos) {
    entity e = {0};
    static uint32_t next_id = 1;
    e.id    = next_id++;
    e.type  = t;
    e.pos   = pos;
    e.hp    = g_info[t].max_hp;
    e.max_hp= g_info[t].max_hp;
    e.alive = 1;
    return e;
}

aabb entity_aabb(const entity *e) {
    float hw = g_info[e->type].width * 0.5f;
float h  = g_info[e->type].height;
;
