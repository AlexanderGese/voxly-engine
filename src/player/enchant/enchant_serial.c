#include "enchant_serial.h"
#include "enchant_set.h"
#include "enchant_registry.h"

size_t enchant_serial_size(const enchant_set *s) {
    int n = s ? s->count : 0;
    return (size_t)2 + (size_t)2 * (size_t)n;
}

size_t enchant_serial_write(const enchant_set *s, uint8_t *buf, size_t cap) {
    if (!buf) return 0;
    size_t need = enchant_serial_size(s);
    if (cap < need) return 0;

    size_t o = 0;
    buf[o++] = ENCHANT_SERIAL_VERSION;
    int n = s ? s->count : 0;
    if (n > ENCHANT_MAX_ON_ITEM) n = ENCHANT_MAX_ON_ITEM;
    buf[o++] = (uint8_t)n;
    for (int i = 0; i < n; ++i) {
        buf[o++] = (uint8_t)s->entry[i].id;
        buf[o++] = (uint8_t)s->entry[i].level;
    }
    return o;
}

size_t enchant_serial_read(enchant_set *out, const uint8_t *buf, size_t len) {
    if (out) enchant_set_clear(out);
    if (!buf || len < 2) return 0;

    size_t o = 0;
    uint8_t ver = buf[o++];
    if (ver != ENCHANT_SERIAL_VERSION) {
        // we only know one version. don't guess at a future layout.
        return 0;
    }
    uint8_t n = buf[o++];
    if (len < (size_t)2 + (size_t)2 * (size_t)n) return 0; // truncated blob

    for (uint8_t i = 0; i < n; ++i) {
        uint8_t id  = buf[o++];
        uint8_t lvl = buf[o++];
        // enchant_set_put validates the id against the registry and clamps the
        // level, so an unknown id just quietly fails to insert. exactly the
        // forward-compat behavior we want.
        if (out) enchant_set_put(out, (enchant_id)id, lvl);
    }
    return o;
}
