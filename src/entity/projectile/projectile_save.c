#include "projectile_save.h"

#include <string.h>

// little-endian cursor writers. the engine's save_v2 does the same hand-rolled
// thing; we keep it local so the projectile module has no link dep on the world
// serializer. no alignment assumptions, no endianness surprises across saves.

typedef struct { uint8_t *p; size_t cap; size_t off; int ok; } wcur;
typedef struct { const uint8_t *p; size_t len; size_t off; int ok; } rcur;

static void w_u32(wcur *c, uint32_t v) {
    if (c->off + 4 > c->cap) { c->ok = 0; return; }
    c->p[c->off++] = (uint8_t)(v & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 8) & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 16) & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 24) & 0xff);
}
static void w_i32(wcur *c, int32_t v) { w_u32(c, (uint32_t)v); }
static void w_f32(wcur *c, float f)   { uint32_t u; memcpy(&u, &f, 4); w_u32(c, u); }
static void w_u8 (wcur *c, uint8_t v) {
    if (c->off + 1 > c->cap) { c->ok = 0; return; }
    c->p[c->off++] = v;
}

static uint32_t r_u32(rcur *c) {
    if (c->off + 4 > c->len) { c->ok = 0; return 0; }
    uint32_t v = (uint32_t)c->p[c->off]
               | ((uint32_t)c->p[c->off + 1] << 8)
               | ((uint32_t)c->p[c->off + 2] << 16)
               | ((uint32_t)c->p[c->off + 3] << 24);
    c->off += 4;
    return v;
}
static int32_t r_i32(rcur *c) { return (int32_t)r_u32(c); }
static float   r_f32(rcur *c) { uint32_t u = r_u32(c); float f; memcpy(&f, &u, 4); return f; }
static uint8_t r_u8 (rcur *c) {
    if (c->off + 1 > c->len) { c->ok = 0; return 0; }
    return c->p[c->off++];
}

// bytes per stuck record. keep in sync with the writer below.
#define PROJ_REC_BYTES (1 + 3*4 + 3*4 + 3*4 + 4 + 4)

static int is_persisted(const projectile *p) {
    return p->state == PROJ_STATE_STUCK;
}

size_t projectile_save_size(const projectile_pool *pool) {
    size_t n = 0;
    for (int i = 0; i < PROJECTILE_POOL_CAP; i++)
        if (is_persisted(&pool->slots[i])) n++;
    return 12 + n * PROJ_REC_BYTES;     // header (3*u32) + records
}

size_t projectile_save_write(const projectile_pool *pool,
                             uint8_t *out, size_t out_cap) {
    size_t need = projectile_save_size(pool);
    if (!out || out_cap < need) return 0;

    wcur c = { out, out_cap, 0, 1 };

    uint32_t count = 0;
    for (int i = 0; i < PROJECTILE_POOL_CAP; i++)
        if (is_persisted(&pool->slots[i])) count++;

    w_u32(&c, PROJECTILE_SAVE_MAGIC);
    w_u32(&c, PROJECTILE_SAVE_VERSION);
    w_u32(&c, count);

    for (int i = 0; i < PROJECTILE_POOL_CAP; i++) {
        const projectile *p = &pool->slots[i];
        if (!is_persisted(p)) continue;
        w_u8 (&c, (uint8_t)p->kind);
        w_i32(&c, p->stuck_bx);
        w_i32(&c, p->stuck_by);
        w_i32(&c, p->stuck_bz);
        w_f32(&c, p->stuck_off.x);
        w_f32(&c, p->stuck_off.y);
        w_f32(&c, p->stuck_off.z);
        w_f32(&c, p->forward.x);
        w_f32(&c, p->forward.y);
        w_f32(&c, p->forward.z);
        w_f32(&c, p->age);
        w_i32(&c, p->owner_id);
    }

    return c.ok ? c.off : 0;
}

int projectile_save_read(projectile_pool *pool,
                         const uint8_t *data, size_t len) {
    if (!data || len < 12) return -1;

    rcur c = { data, len, 0, 1 };
    uint32_t magic = r_u32(&c);
    uint32_t ver   = r_u32(&c);
    uint32_t count = r_u32(&c);
    if (!c.ok || magic != PROJECTILE_SAVE_MAGIC) return -1;
    if (ver != PROJECTILE_SAVE_VERSION) return -1;     // no migration yet

    int restored = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t  kind    = r_u8(&c);
        int32_t  bx      = r_i32(&c);
        int32_t  by      = r_i32(&c);
        int32_t  bz      = r_i32(&c);
        float    ox      = r_f32(&c);
        float    oy      = r_f32(&c);
        float    oz      = r_f32(&c);
        float    fx      = r_f32(&c);
        float    fy      = r_f32(&c);
        float    fz      = r_f32(&c);
        float    age     = r_f32(&c);
        int32_t  owner   = r_i32(&c);
        if (!c.ok) return -1;          // truncated blob, bail honestly

        // the loaded id is fresh — we never persist ids, they're per-session.
        // borrow the slot index as a cheap unique-ish seed; +1 so it's never 0.
        projectile *p = projectile_pool_alloc(pool, (uint32_t)(i + 1));
        if (!p) break;                 // pool full; drop the rest silently

        p->kind = (projectile_kind)kind;
        p->state = PROJ_STATE_STUCK;
        p->stuck_bx = bx; p->stuck_by = by; p->stuck_bz = bz;
        p->stuck_off = (vec3){ ox, oy, oz };
        p->forward = (vec3){ fx, fy, fz };
        p->age = age;
        p->owner_id = owner;
        p->pos = (vec3){ (float)bx + ox, (float)by + oy, (float)bz + oz };
        restored++;
    }
    return restored;
}
