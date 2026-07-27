#include "ecs_snapshot.h"

#include <stdlib.h>
#include <string.h>

#include "../../config.h"
#include "../../util/log.h"

// ---- blob plumbing ---------------------------------------------------------

void ecs_blob_init(ecs_blob *b) {
    b->data = NULL;
    b->len  = 0;
    b->cap  = 0;
}

void ecs_blob_free(ecs_blob *b) {
    free(b->data);
    ecs_blob_init(b);
}

static void blob_write(ecs_blob *b, const void *src, size_t n) {
    if (b->len + n > b->cap) {
        size_t newcap = b->cap ? b->cap * 2 : 256;
        while (newcap < b->len + n) newcap *= 2;
        b->data = realloc(b->data, newcap);
        b->cap  = newcap;
    }
    memcpy(b->data + b->len, src, n);
    b->len += n;
}

static void blob_write_u32(ecs_blob *b, uint32_t v) { blob_write(b, &v, 4); }
static void blob_write_u64(ecs_blob *b, uint64_t v) { blob_write(b, &v, 8); }

// cursor-based reader. all reads bounds-check against `len`; on overrun they
// return 0 and leave a flag so the caller bails. no partial-load nonsense.
typedef struct {
    const uint8_t *p;
    size_t         len;
    size_t         off;
    int            bad;
} blob_reader;

static int rd_bytes(blob_reader *r, void *dst, size_t n) {
    if (r->bad || r->off + n > r->len) { r->bad = 1; return 0; }
    memcpy(dst, r->p + r->off, n);
    r->off += n;
    return 1;
}
static uint32_t rd_u32(blob_reader *r) { uint32_t v = 0; rd_bytes(r, &v, 4); return v; }
static uint64_t rd_u64(blob_reader *r) { uint64_t v = 0; rd_bytes(r, &v, 8); return v; }

// ---- save ------------------------------------------------------------------

uint32_t ecs_snapshot_save(ecs_world *w, ecs_blob *out) {
    out->len = 0;

    blob_write_u32(out, ECS_SNAP_MAGIC);
    blob_write_u32(out, SAVE_VERSION);
    blob_write_u32(out, ECS_CMP_COUNT);

    // reserve a slot for the entity count; we backpatch it once we know it.
    size_t count_at = out->len;
    blob_write_u32(out, 0);

    uint32_t written = 0;
    // walk the pool's slots directly. iterating one store would miss entities
    // that lack that component, so we go off the alive flags.
    for (uint32_t i = 0; i < w->pool.hiwater; i++) {
        if (!w->pool.alive[i]) continue;
        ecs_entity e = ecs_entity_make(i, w->pool.gen[i]);
        ecs_signature sig = w->pool.sig[i];

        blob_write_u32(out, (uint32_t)e);
        blob_write_u64(out, sig);

        // components in id order so the loader can replay deterministically.
        for (int c = 0; c < ECS_CMP_COUNT; c++) {
            if (!ecs_sig_has(sig, (ecs_component_id)c)) continue;
            void *blob = ecs_store_get(&w->stores[c], e);
            size_t sz  = ecs_component_size((ecs_cmp)c);
            if (blob) blob_write(out, blob, sz);
            else {
                // signature claimed it but the store disagrees -- shouldnt
                // happen, but pad with zeros so offsets stay aligned.
                static const uint8_t zeros[64] = {0};
                blob_write(out, zeros, sz);
                LOGW("ecs snapshot: %s missing on entity %u",
                     ecs_component_name((ecs_cmp)c), i);
            }
        }
        written++;
    }

    memcpy(out->data + count_at, &written, 4);   // backpatch
    return written;
}

// ---- load ------------------------------------------------------------------

int ecs_snapshot_load(ecs_world *w, const uint8_t *data, size_t len) {
    blob_reader r = { data, len, 0, 0 };

    uint32_t magic = rd_u32(&r);
    uint32_t ver   = rd_u32(&r);
    uint32_t ncmp  = rd_u32(&r);
    uint32_t nent  = rd_u32(&r);
    if (r.bad) return 0;

    if (magic != ECS_SNAP_MAGIC) {
        LOGE("ecs snapshot: bad magic %08x", magic);
        return 0;
    }
    if (ver != SAVE_VERSION) {
        LOGE("ecs snapshot: version %u != %u, refusing", ver, SAVE_VERSION);
        return 0;
    }
    if (ncmp != ECS_CMP_COUNT) {
        // component layout changed under us; a migration table would go here.
        LOGE("ecs snapshot: component count %u != %u", ncmp, ECS_CMP_COUNT);
        return 0;
    }

    ecs_world_clear(w);

    for (uint32_t n = 0; n < nent; n++) {
        uint32_t handle = rd_u32(&r);
        uint64_t sig    = rd_u64(&r);
        if (r.bad) { LOGE("ecs snapshot: truncated at entity %u", n); return 0; }

        ecs_entity e = ecs_pool_restore(&w->pool,
                                        ecs_entity_index(handle),
                                        ecs_entity_gen(handle));
        if (e == ECS_NULL) return 0;
        w->pool.sig[ecs_entity_index(e)] = sig;

        for (int c = 0; c < ECS_CMP_COUNT; c++) {
            if (!ecs_sig_has(sig, (ecs_component_id)c)) continue;
            size_t sz = ecs_component_size((ecs_cmp)c);
            // add a zeroed slot then read straight into it -- saves a bounce
            // through a temp buffer for the bigger components.
            void *slot = ecs_store_add(&w->stores[c], e, NULL);
            if (!rd_bytes(&r, slot, sz)) {
                LOGE("ecs snapshot: truncated component %s", ecs_component_name((ecs_cmp)c));
                return 0;
            }
        }
    }

    ecs_pool_reseed(&w->pool);
    return 1;
}
