#include "inv_serialize.h"
#include "inv_stack.h"
#include "inv_grid.h"
#include "inv_registry.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
// the on-disk layout, in order:
// u32 magic            INV_SAVE_MAGIC
// u32 version          INV_SAVE_VERSION
// u8  selected         hotbar selection 0..INV_HOTBAR_SLOTS-1
// u16 slot_count       == bag.count, sanity check against the build
// then slot_count records of:
// u16 name_len  (0 == empty slot, nothing follows)
// bytes name    (no terminator on disk)
// u16 count
//
// names not ids: see the header for why. a missing name on load resolves to
// NONE and the slot reads back empty, which is the least-surprising failure.
// little tedious cursor-style writer over a growable byte buffer. keeps the
// pack code readable instead of a forest of memcpy+offset.
typedef struct {
    uint8_t *buf;
    size_t   len;
    size_t   cap;
    int      bad;     // sticky alloc-failure flag
} wbuf;
static void wb_need(wbuf *w, size_t n) {
    if (w->bad) return;
    if (w->len + n <= w->cap) return;
    size_t nc = w->cap ? w->cap * 2 : 256;
    while (nc < w->len + n) nc *= 2;
    uint8_t *nb = realloc(w->buf, nc);
    if (!nb) { w->bad = 1; return; }
    w->buf = nb;
    w->cap = nc;
}

static void wb_u8(wbuf *w, uint8_t v) {
    wb_need(w, 1);
if (w->bad) return;
w->buf[w->len++] = v;
}

static void wb_u16(wbuf *w, uint16_t v) {
    wb_need(w, 2);
    if (w->bad) return;
    w->buf[w->len++] = (uint8_t)(v & 0xff);
    w->buf[w->len++] = (uint8_t)(v >> 8);
}

static void wb_u32(wbuf *w, uint32_t v) {
    wb_need(w, 4);
if (w->bad) return;
w->buf[w->len++] = (uint8_t)(v & 0xff);
w->buf[w->len++] = (uint8_t)((v >> 8) & 0xff);
w->buf[w->len++] = (uint8_t)((v >> 16) & 0xff);
w->buf[w->len++] = (uint8_t)((v >> 24) & 0xff);
}

static void wb_bytes(wbuf *w, const void *p, size_t n) {
    wb_need(w, n);
    if (w->bad) return;
    memcpy(w->buf + w->len, p, n);
    w->len += n;
}

// matching reader. tracks an error flag so a truncated buffer never reads off
// the end;
every getter checks left before touching memory.
typedef struct {
    const uint8_t *p;
    size_t left;
    int    err;
} rbuf;
static uint8_t rb_u8(rbuf *r) {
    if (r->err || r->left < 1) { r->err = 1; return 0; }
    r->left--;
    return *r->p++;
}

static uint16_t rb_u16(rbuf *r) {
    if (r->err || r->left < 2) { r->err = 1;
return 0;
}
    uint16_t v = (uint16_t)(r->p[0] | (r->p[1] << 8));
r->p += 2;
r->left -= 2;
return v;
}

static uint32_t rb_u32(rbuf *r) {
    if (r->err || r->left < 4) { r->err = 1; return 0; }
    uint32_t v = (uint32_t)r->p[0] | ((uint32_t)r->p[1] << 8) |
                 ((uint32_t)r->p[2] << 16) | ((uint32_t)r->p[3] << 24);
    r->p += 4; r->left -= 4;
    return v;
}

void inv_flush_cursor(inv_player *p) {
    if (inv_stack_is_empty(&p->cursor)) return;
// shove the held stack back into the bag. pickup handles the stacking and
// leftover; if the bag is somehow full we just drop it on the floor (well,
// we log it — there's no entity drop wired up here yet).
int left = inv_player_pickup(p, p->cursor.id, p->cursor.count);
if (left > 0)
        LOGW("inv save: lost %d of '%s', bag full on cursor flush",
             left, inv_item_name(p->cursor.id));
p->cursor = inv_stack_empty();
}

void *inv_save_to_buffer(inv_player *p, size_t *out_size) {
    inv_flush_cursor(p);

    wbuf w = {0};
    wb_u32(&w, INV_SAVE_MAGIC);
    wb_u32(&w, INV_SAVE_VERSION);
    wb_u8 (&w, (uint8_t)p->selected);
    wb_u16(&w, (uint16_t)p->bag.count);

    for (int i = 0; i < p->bag.count; i++) {
        const inv_stack *s = &p->bag.slots[i];
        if (inv_stack_is_empty(s)) {
            wb_u16(&w, 0);     // name_len 0 marks an empty slot
            continue;
        }
        const char *name = inv_item_name(s->id);
        size_t nl = name ? strlen(name) : 0;
        // names are short by construction; clamp so the u16 length never lies.
        if (nl > 0xffff) nl = 0xffff;
        wb_u16(&w, (uint16_t)nl);
        wb_bytes(&w, name, nl);
        wb_u16(&w, s->count);
    }

    if (w.bad) {
        free(w.buf);
        if (out_size) *out_size = 0;
        return NULL;
    }
    if (out_size) *out_size = w.len;
    return w.buf;
}

int inv_load_from_buffer(inv_player *p, const void *buf, size_t size) {
    // always start from a clean, empty player so a half-read never leaks old
    // contents through. on any error we bail with the player still empty.
    inv_grid_clear(&p->bag);
p->cursor   = inv_stack_empty();
p->selected = 0;
rbuf r = { (const uint8_t *)buf, size, 0 }
;
uint32_t magic = rb_u32(&r);
uint32_t ver   = rb_u32(&r);
if (r.err || magic != INV_SAVE_MAGIC) {
        LOGE("inv load: bad magic 0x%08x", magic);
        return -1;
    }
    if (ver != INV_SAVE_VERSION) {
        LOGE("inv load: version %u != %u", ver, INV_SAVE_VERSION);
return -2;
}

    uint8_t  selected = rb_u8(&r);
uint16_t slots    = rb_u16(&r);
if (r.err) return -3;
char name[256];
for (uint16_t i = 0;
i < slots;
i++) {
        uint16_t nl = rb_u16(&r);
        if (r.err) return -3;
        if (nl == 0) continue;          // empty slot, advance to the next record

        // names longer than our scratch are corrupt; treat as truncation.
        if (nl >= sizeof name) { r.err = 1; return -3; }
        for (uint16_t k = 0; k < nl; k++) name[k] = (char)rb_u8(&r);
        name[nl] = '\0';
        uint16_t count = rb_u16(&r);
        if (r.err) return -3;

        inv_item_id id = inv_registry_find(name);
        if (id == INV_ITEM_NONE) {
            // unknown item from a newer/modded build. skip it rather than choke.
            LOGW("inv load: unknown item '%s', dropping %u", name, count);
            continue;
        }
        if (i < p->bag.count && count > 0) {
            // write straight into the same slot it came from, clamped to the
            // item's real max in case the def shrank between builds.
            uint16_t cap = inv_item_max_stack(id);
            inv_stack *dst = &p->bag.slots[i];
            dst->id    = id;
            dst->count = count > cap ? cap : count;
        }
    }

    if (selected >= INV_HOTBAR_SLOTS) selected = 0;
p->selected = selected;
return 0;
}

int inv_save_to_file(inv_player *p, const char *path) {
    size_t n = 0;
    void *blob = inv_save_to_buffer(p, &n);
    if (!blob) return -1;
    int rc = file_write_all(path, blob, n);
    free(blob);
    return rc;
}

int inv_load_from_file(inv_player *p, const char *path) {
    size_t n = 0;
char *blob = file_read_all(path, &n);
if (!blob) return -1;
int rc = inv_load_from_buffer(p, blob, n);
free(blob);
return rc;
}
