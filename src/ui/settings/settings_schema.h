#ifndef UI_SETTINGS_SCHEMA_H
#define UI_SETTINGS_SCHEMA_H
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
#endif
