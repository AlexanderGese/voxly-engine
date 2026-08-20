#ifndef UI_SETTINGS_VALUE_H
#define UI_SETTINGS_VALUE_H

// a single tagged setting value. the menu stores everything as one of these so
// the slider / toggle / serializer code can be generic. floats and ints share
// the same float payload (ints are stored exactly representable, we round on the
// way out); bools and enums live in the int payload.
//
// every value carries its own range so clamping and the 0..1 slider mapping are
// self-contained — the schema fills these in once and the model copies them.

#include "settings_types.h"

typedef struct {
    settings_opt_kind kind;
    float f;        // float payload (also holds int as a float for slider math)
    int   i;        // int / bool / enum index payload
    float lo, hi;   // inclusive range (floats) or [lo,hi] index range (enum)
    float step;     // quantum for stepped sliders / spinners; 0 = continuous
} settings_value;

// constructors. ranges are validated lightly (lo<=hi swapped if reversed).
settings_value settings_value_float(float v, float lo, float hi, float step);
settings_value settings_value_int(int v, int lo, int hi, int step);
settings_value settings_value_bool(int v);
settings_value settings_value_enum(int v, int count);

// clamp the payload back into range. enum wraps would be surprising, so this
// clamps enums too; the cycler does its own wrap when the user asks for it.
void settings_value_clamp(settings_value *v);

// 0..1 normalized position of the value within its range, for slider geometry.
// bools map to their 0/1 directly; enums spread their indices evenly.
float settings_value_norm(const settings_value *v);

// set the value from a 0..1 slider position, snapping to `step` if stepped.
// returns 1 if the stored value actually changed (so callers can flag dirty).
int settings_value_set_norm(settings_value *v, float t);

// nudge by +/- one step (or a sensible default). used by keyboard left/right and
// the spinner arrows. returns 1 if it changed.
int settings_value_step(settings_value *v, int dir);

// cycle an enum / flip a bool. dir is +1 / -1. returns 1 if it changed.
int settings_value_cycle(settings_value *v, int dir);

// equality with a small epsilon for the float payload. used by dirty tracking.
int settings_value_equal(const settings_value *a, const settings_value *b);

// render the value into `out` for the readout text, using the option's kind and
// (for enums) the supplied choice labels. always null-terminates.
void settings_value_format(const settings_value *v, const char *const *choices,
                           int choice_count, char *out, size_t out_sz);

#endif
