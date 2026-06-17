#include "farming_serialize.h"
#include <stdlib.h>
#include <string.h>

// on-disk record sizes. we write fields explicitly rather than memcpy'ing the
// structs so the format survives padding/layout changes to the in-memory types.
// tile: wx,wy,wz (3*i32) + hydration,trample (2*u8) + dry_timer (f32) = 18
// crop: wx,wy,wz (3*i32) + kind,stage,flags (3*u8) + growth_accum (f32)
// + planted_tick (u32) = 23
#define REC_TILE   18
#define REC_CROP   23
#define HEADER_LEN 24

// --- little-endian cursor writer over a fixed buffer ---------------------
typedef struct { uint8_t *p; size_t cap; size_t off; } wcur;

static void w_u8(wcur *c, uint8_t v) {
    if (c->off + 1 <= c->cap) c->p[c->off] = v;
    c->off += 1;
}
static void w_u16(wcur *c, uint16_t v) {
    w_u8(c, (uint8_t)(v & 0xff));
    w_u8(c, (uint8_t)(v >> 8));
}
static void w_u32(wcur *c, uint32_t v) {
    w_u16(c, (uint16_t)(v & 0xffff));
    w_u16(c, (uint16_t)(v >> 16));
}
static void w_i32(wcur *c, int32_t v) { w_u32(c, (uint32_t)v); }
static void w_f32(wcur *c, float v) {
    uint32_t bits;
    memcpy(&bits, &v, sizeof bits);
    w_u32(c, bits);
}

// --- little-endian cursor reader -----------------------------------------
typedef struct { const uint8_t *p; size_t len; size_t off; } rcur;

static int r_u8(rcur *c, uint8_t *out) {
    if (c->off + 1 > c->len) return -1;
    *out = c->p[c->off++];
    return 0;
}
static int r_u16(rcur *c, uint16_t *out) {
    uint8_t a, b;
    if (r_u8(c, &a) || r_u8(c, &b)) return -1;
    *out = (uint16_t)(a | (b << 8));
    return 0;
}
static int r_u32(rcur *c, uint32_t *out) {
    uint16_t a, b;
    if (r_u16(c, &a) || r_u16(c, &b)) return -1;
    *out = (uint32_t)a | ((uint32_t)b << 16);
    return 0;
}
static int r_i32(rcur *c, int32_t *out) {
    uint32_t v;
    if (r_u32(c, &v)) return -1;
    *out = (int32_t)v;
    return 0;
}
static int r_f32(rcur *c, float *out) {
    uint32_t v;
    if (r_u32(c, &v)) return -1;
    memcpy(out, &v, sizeof *out);
    return 0;
}

size_t farming_serialize_size(const farming_field *f) {
    size_t tiles = farming_field_tile_count(f);
    size_t crops = farming_field_crop_count(f);
    return HEADER_LEN + tiles * REC_TILE + crops * REC_CROP;
}

uint8_t *farming_serialize_write(const farming_field *f, size_t *out_len) {
    size_t total = farming_serialize_size(f);
    uint8_t *buf = (uint8_t *)malloc(total ? total : 1);
    if (!buf) return NULL;

    wcur c = { buf, total, 0 };

    w_u32(&c, FARMING_SAVE_MAGIC);
    w_u16(&c, FARMING_SAVE_VERSION);
    w_u16(&c, 0); // reserved
    w_u32(&c, f->seed);
    w_u32(&c, f->tick);
    w_u32(&c, (uint32_t)farming_field_tile_count(f));
    w_u32(&c, (uint32_t)farming_field_crop_count(f));

    // tiles. we const-cast the maps to iterate; the iterator only reads.
    hm_iter it;
    uint64_t k;
    void *v;

    hm_iter_init(&it, &f->tiles);
    while (hm_iter_next(&it, &k, &v)) {
        const farming_tile *t = (const farming_tile *)v;
        w_i32(&c, t->wx);
        w_i32(&c, t->wy);
        w_i32(&c, t->wz);
        w_u8(&c, t->hydration);
        w_u8(&c, t->trample);
        w_f32(&c, t->dry_timer);
    }

    hm_iter_init(&it, &f->crops);
    while (hm_iter_next(&it, &k, &v)) {
        const farming_crop *cr = (const farming_crop *)v;
        w_i32(&c, cr->wx);
        w_i32(&c, cr->wy);
        w_i32(&c, cr->wz);
        w_u8(&c, cr->kind);
        w_u8(&c, cr->stage);
        w_u8(&c, cr->flags);
        w_f32(&c, cr->growth_accum);
        w_u32(&c, cr->planted_tick);
    }

    if (out_len) *out_len = c.off;
    return buf;
}

// drop every record currently held so a load starts from a clean field. we
// reuse the field's own free pass by re-init'ing the maps in place.
static void clear_records(farming_field *f) {
    hm_iter it;
    uint64_t k;
    void *v;

    hm_iter_init(&it, &f->tiles);
    while (hm_iter_next(&it, &k, &v)) free(v);
    hashmap_free(&f->tiles);
    hashmap_init(&f->tiles, 64);

    hm_iter_init(&it, &f->crops);
    while (hm_iter_next(&it, &k, &v)) free(v);
    hashmap_free(&f->crops);
    hashmap_init(&f->crops, 64);
}

// same coord packer the field uses for its keys. duplicated here so the loader
// can re-key records without exposing the field's internal helper.
static uint64_t key_of(int x, int y, int z) {
    uint64_t ux = (uint64_t)(uint32_t)(x + (1 << 20)) & 0x1FFFFFull;
    uint64_t uy = (uint64_t)(uint32_t)(y + (1 << 20)) & 0x1FFFFFull;
    uint64_t uz = (uint64_t)(uint32_t)(z + (1 << 20)) & 0x1FFFFFull;
    return ux | (uy << 21) | (uz << 42);
}

int farming_serialize_read(farming_field *f, const uint8_t *data, size_t len) {
    rcur c = { data, len, 0 };

    uint32_t magic, seed, tick, tile_n, crop_n;
    uint16_t ver, rsv;
    if (r_u32(&c, &magic) || magic != FARMING_SAVE_MAGIC) return -1;
    if (r_u16(&c, &ver)   || ver != FARMING_SAVE_VERSION) return -2;
    if (r_u16(&c, &rsv))   return -1;
    if (r_u32(&c, &seed))  return -1;
    if (r_u32(&c, &tick))  return -1;
    if (r_u32(&c, &tile_n)) return -1;
    if (r_u32(&c, &crop_n)) return -1;

    // sanity: do the declared counts even fit in whats left?
    if ((size_t)tile_n * REC_TILE + (size_t)crop_n * REC_CROP > len - c.off)
        return -3;

    clear_records(f);
    f->seed = seed;
    f->tick = tick;

    for (uint32_t i = 0; i < tile_n; i++) {
        farming_tile *t = (farming_tile *)malloc(sizeof *t);
        if (!t) return -4;
        uint8_t hyd, tr;
        if (r_i32(&c, &t->wx) || r_i32(&c, &t->wy) || r_i32(&c, &t->wz) ||
            r_u8(&c, &hyd) || r_u8(&c, &tr) || r_f32(&c, &t->dry_timer)) {
            free(t);
            return -5;
        }
        t->hydration = hyd;
        t->trample = tr;
        t->has_crop = 0; // re-derived from the crop stream below
        t->_pad = 0;
        hashmap_put(&f->tiles, key_of(t->wx, t->wy, t->wz), t);
    }

    for (uint32_t i = 0; i < crop_n; i++) {
        farming_crop *cr = (farming_crop *)malloc(sizeof *cr);
        if (!cr) return -4;
        if (r_i32(&c, &cr->wx) || r_i32(&c, &cr->wy) || r_i32(&c, &cr->wz) ||
            r_u8(&c, &cr->kind) || r_u8(&c, &cr->stage) || r_u8(&c, &cr->flags) ||
            r_f32(&c, &cr->growth_accum) || r_u32(&c, &cr->planted_tick)) {
            free(cr);
            return -5;
        }
        cr->_pad = 0;
        // crops key on their farmland coord (one below the plant).
        uint64_t tk = key_of(cr->wx, cr->wy - 1, cr->wz);
        hashmap_put(&f->crops, tk, cr);
        farming_tile *t = (farming_tile *)hashmap_get(&f->tiles, tk);
        if (t) t->has_crop = 1;
    }

    return 0;
}
