#include "settings_diff.h"
#include "settings_schema.h"

#include <stdio.h>
#include <string.h>

// format one value the same way the menu readout does, so the diff matches what
// the user saw on screen. enums get their choice label, bools on/off, etc.
static void fmt_value(const settings_value *v, settings_id id,
                      char *out, size_t cap) {
    const settings_opt *opt = settings_schema_opt(id);
    if (opt->kind == SETTINGS_OPT_ENUM)
        settings_value_format(v, opt->choices, opt->choice_count, out, cap);
    else
        settings_value_format(v, NULL, 0, out, cap);
}

// copy with hard truncation + guaranteed terminator. snprintf with "%s" would
// do, but this keeps the intent obvious at the call sites.
static void copy_str(char *dst, size_t cap, const char *src) {
    if (cap == 0) return;
    size_t n = strlen(src);
    if (n >= cap) n = cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

int settings_diff_collect(settings_diff *d, const settings_model *m) {
    d->count = 0;
    for (int i = 0; i < SETTINGS_ID_COUNT; i++) {
        settings_id id = (settings_id)i;
        if (!settings_model_field_dirty(m, id)) continue;

        settings_diff_row *row = &d->rows[d->count++];
        row->id = id;
        copy_str(row->label, sizeof row->label, settings_id_label(id));

        const settings_value *live = settings_model_live(m, id);
        // work copy lives behind the non-const accessor; cast away const just to
        // read it — collect never writes, so this is safe.
        const settings_value *work =
            settings_model_work((settings_model *)m, id);

        fmt_value(live, id, row->old_val, sizeof row->old_val);
        fmt_value(work, id, row->new_val, sizeof row->new_val);
    }
    return d->count;
}

size_t settings_diff_format(const settings_diff *d, char *out, size_t cap) {
    if (out && cap) out[0] = '\0';
    size_t off = 0;
    for (int i = 0; i < d->count; i++) {
        const settings_diff_row *r = &d->rows[i];
        int n = snprintf(out + (off < cap ? off : cap),
                         off < cap ? cap - off : 0,
                         "%s: %s -> %s\n", r->label, r->old_val, r->new_val);
        if (n < 0) break;
        off += (size_t)n;
    }
    return off;
}

int settings_diff_any(const settings_model *m) {
    return m->dirty_count > 0;
}
