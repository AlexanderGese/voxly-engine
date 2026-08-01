#include "projectile_save.h"
#include <string.h>
typedef struct { uint8_t *p; size_t cap; size_t off; int ok; } wcur;
typedef struct { const uint8_t *p; size_t len; size_t off; int ok; } rcur;
static void w_u32(wcur *c, uint32_t v) {
    if (c->off + 4 > c->cap) { c->ok = 0; return; }
    c->p[c->off++] = (uint8_t)(v & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 8) & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 16) & 0xff);
    c->p[c->off++] = (uint8_t)((v >> 24) & 0xff);
}
static void w_i32(wcur *c, int32_t v) { w_u32(c, (uint32_t)v);
}
static void w_f32(wcur *c, float f)   { uint32_t u; memcpy(&u, &f, 4); w_u32(c, u); }
static void w_u8 (wcur *c, uint8_t v) {
    if (c->off + 1 > c->cap) { c->ok = 0;
return;
}
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
static int32_t r_i32(rcur *c) { return (int32_t)r_u32(c);
}
static float   r_f32(rcur *c) { uint32_t u = r_u32(c); float f; memcpy(&f, &u, 4); return f; }
static uint8_t r_u8 (rcur *c) {
    if (c->off + 1 > c->len) { c->ok = 0;
return 0;
}
    return c->p[c->off++];
}

// bytes per stuck record. keep in sync with the writer below.
#define PROJ_REC_BYTES (1 + 3*4 + 3*4 + 3*4 + 4 + 4)

static int is_persisted(const projectile *p) {
    return p->state == PROJ_STATE_STUCK;
}

size_t projectile_save_size(const projectile_pool *pool) {
    size_t n = 0;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
i++)
        if (is_persisted(&pool->slots[i])) n++;
return 12 + n * PROJ_REC_BYTES;
;
uint32_t magic = r_u32(&c);
uint32_t ver   = r_u32(&c);
uint32_t count = r_u32(&c);
if (!c.ok || magic != PROJECTILE_SAVE_MAGIC) return -1;
if (ver != PROJECTILE_SAVE_VERSION) return -1;
int restored = 0;
for (uint32_t i = 0;
i < count;
}
