#ifndef UI_SETTINGS_SCHEMA_H
#define UI_SETTINGS_SCHEMA_H

// the static description of every setting: its kind, default, range, and (for
// enums) the choice labels. the model reads this once at init to seed all three
// value copies, and the layout reads it each frame for ranges + readout labels.
//
// keeping the schema separate from the live values means the menu is fully
// data-driven: add a row here + an id in settings_types and a new control just
// appears under its tab, no per-widget code.

#include "settings_types.h"
#include "settings_value.h"

#define SETTINGS_MAX_CHOICES 6

typedef struct {
    settings_id        id;
    settings_opt_kind  kind;
    // default value, plus range/step folded into a settings_value template. the
    // model copies the template and only overwrites the payload when loading.
    settings_value     def;
    // enum choices, NULL for non-enum. count must match the def's index range.
    const char        *choices[SETTINGS_MAX_CHOICES];
    int                choice_count;
} settings_opt;

// the whole table, in id order. returns a pointer to a static array of length
// SETTINGS_ID_COUNT. row i describes settings_id i.
const settings_opt *settings_schema(void);

// convenience: the row for one id (bounds-checked, never NULL).
const settings_opt *settings_schema_opt(settings_id id);

// count of options belonging to `tab`. used to pre-size the per-tab view.
int settings_schema_tab_count(settings_tab tab);

// fill `out` (capacity `cap`) with the ids under `tab`, in id order. returns how
// many were written (<= cap).
int settings_schema_tab_ids(settings_tab tab, settings_id *out, int cap);

#endif
