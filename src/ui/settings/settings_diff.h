#ifndef UI_SETTINGS_DIFF_H
#define UI_SETTINGS_DIFF_H

// enumerates the pending changes between the model's work copy and the running
// live values, formatted as human-readable "label: old -> new" rows. used two
// ways: by a "you have unsaved changes" confirm prompt, and by the debug console
// to dump what the user is about to apply. it never mutates the model.
//
// the strings are pre-rendered into a fixed-size table so the caller can paint
// them without re-walking the schema or worrying about lifetime.

#include "settings_model.h"

#define SETTINGS_DIFF_LABEL  48
#define SETTINGS_DIFF_VALUE  24

typedef struct {
    settings_id id;
    char        label[SETTINGS_DIFF_LABEL];
    char        old_val[SETTINGS_DIFF_VALUE];   // current live, formatted
    char        new_val[SETTINGS_DIFF_VALUE];   // pending work, formatted
} settings_diff_row;

typedef struct {
    settings_diff_row rows[SETTINGS_ID_COUNT];
    int               count;
} settings_diff;

// fill `d` with one row per dirty field, in id order. returns the row count
// (== d->count). a clean model yields zero rows.
int settings_diff_collect(settings_diff *d, const settings_model *m);

// write the diff as a multi-line "label: old -> new" block into `out` (capacity
// `cap`, always terminated). returns the would-be length like snprintf. handy
// for the console dump / logging.
size_t settings_diff_format(const settings_diff *d, char *out, size_t cap);

// true if the model has any pending change. cheap wrapper for prompt gating.
int settings_diff_any(const settings_model *m);

#endif
