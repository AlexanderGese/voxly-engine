#include "spawnx_serialize.h"
#include <string.h>

// little-endian put/get, same convention as the rest of the save code. assumes
// an LE host, which everything else here already does.
static void put_u8(uint8_t **p, uint8_t v) { (*p)[0] = v; *p += 1; }

static void put_u16(uint8_t **p, uint16_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
    (*p)[1] = (uint8_t)(v >> 8);
    *p += 2;
}

static void put_u32(uint8_t **p, uint32_t v) {
    (*p)[0] = (uint8_t)(v & 0xFF);
    (*p)[1] = (uint8_t)((v >> 8) & 0xFF);
    (*p)[2] = (uint8_t)((v >> 16) & 0xFF);
    (*p)[3] = (uint8_t)((v >> 24) & 0xFF);
    *p += 4;
}

static void put_i32(uint8_t **p, int32_t v) { put_u32(p, (uint32_t)v); }

static uint8_t  get_u8(const uint8_t **p)  { uint8_t v = (*p)[0]; *p += 1; return v; }

static uint16_t get_u16(const uint8_t **p) {
    uint16_t v = (uint16_t)((*p)[0] | ((*p)[1] << 8));
    *p += 2;
    return v;
}

static uint32_t get_u32(const uint8_t **p) {
    uint32_t v = (uint32_t)((*p)[0]) | ((uint32_t)(*p)[1] << 8)
               | ((uint32_t)(*p)[2] << 16) | ((uint32_t)(*p)[3] << 24);
    *p += 4;
    return v;
}

static int32_t get_i32(const uint8_t **p) { return (int32_t)get_u32(p); }

size_t spawnx_serialize_max_bytes(void) {
    // header (4+2+2+2) + all spawner slots (14 each) + all anchor slots (5 each).
    return 10 + (size_t)SPAWNX_MAX_SPAWNERS * 14
              + (size_t)SPAWNX_ANCHOR_MAX  * 5;
}

size_t spawnx_serialize_encode(const spawnx *sx, uint8_t *dst, size_t cap) {
    if (!sx || !dst) return 0;
    if (cap < spawnx_serialize_max_bytes()) return 0;

    uint8_t *p = dst;
    put_u32(&p, SPAWNX_SAVE_MAGIC);
    put_u16(&p, SPAWNX_SAVE_VERSION);

    // backfill the two counts once we know how many we actually wrote.
    uint8_t *sp_count_at = p; put_u16(&p, 0);
    uint8_t *an_count_at = p; put_u16(&p, 0);

    uint16_t sp = 0;
    for (int i = 0; i < SPAWNX_MAX_SPAWNERS; i++) {
        const spawnx_blockspawner *bs = &sx->spawner[i];
        if (!bs->active) continue;
        put_i32(&p, bs->wx);
        put_i32(&p, bs->wy);
        put_i32(&p, bs->wz);
        put_u8(&p, (uint8_t)bs->type);
        put_u8(&p, (uint8_t)bs->biome);
        sp++;
    }

    uint16_t an = 0;
    for (int i = 0; i < SPAWNX_ANCHOR_MAX; i++) {
        const spawnx_anchor *a = &sx->anchors.anchor[i];
        if (a->entity_id == 0 || !a->sticky) continue;   // only sticky persists
        put_u32(&p, a->entity_id);
        put_u8(&p, a->reason);
        an++;
    }

    // backfill.
    sp_count_at[0] = (uint8_t)(sp & 0xFF);
    sp_count_at[1] = (uint8_t)(sp >> 8);
    an_count_at[0] = (uint8_t)(an & 0xFF);
    an_count_at[1] = (uint8_t)(an >> 8);

    return (size_t)(p - dst);
}

size_t spawnx_serialize_decode(spawnx *sx, const uint8_t *src, size_t len) {
    if (!sx || !src) return 0;
    if (len < 10) return 0;

    const uint8_t *p = src;
    if (get_u32(&p) != SPAWNX_SAVE_MAGIC) return 0;
    if (get_u16(&p) != SPAWNX_SAVE_VERSION) return 0;

    uint16_t sp = get_u16(&p);
    uint16_t an = get_u16(&p);
    if (sp > SPAWNX_MAX_SPAWNERS || an > SPAWNX_ANCHOR_MAX) return 0;

    // bounds-check the rest before we commit to reading it.
    size_t need = 10 + (size_t)sp * 14 + (size_t)an * 5;
    if (len < need) return 0;

    // wipe live placed state; loading replaces it wholesale.
    for (int i = 0; i < SPAWNX_MAX_SPAWNERS; i++) sx->spawner[i].active = 0;
    spawnx_anchor_init(&sx->anchors);

    for (int i = 0; i < sp; i++) {
        int32_t wx = get_i32(&p);
        int32_t wy = get_i32(&p);
        int32_t wz = get_i32(&p);
        uint8_t ty = get_u8(&p);
        uint8_t bi = get_u8(&p);
        // reseed from position; the rng stream is derived, never stored.
        spawnx_bs_make(&sx->spawner[i], wx, wy, wz,
                       (entity_type)ty, (biome_id)bi, sx->world_seed);
    }

    for (int i = 0; i < an; i++) {
        uint32_t id = get_u32(&p);
        uint8_t  rs = get_u8(&p);
        spawnx_anchor_add(&sx->anchors, id, (spawnx_source)rs, 1);
    }

    return (size_t)(p - src);
}
