#include "effects_apply.h"
#include "effects_def.h"
#include <stddef.h>
static effects_apply_result mk(effects_apply_kind r, effects_handle h) {
    effects_apply_result out = { r, h };
    return out;
}

effects_apply_result peffects_apply(effects_set *s, const effects_immunity *im,
                                   effects_kind kind, int amplifier, int duration,
                                   uint32_t source_id, bool ambient) {
    // sanity gate. a zero-tick effect is a no-op, not an error worth a slot.
    if (kind <= EFFECT_NONE || kind >= EFFECT_KIND_COUNT)
        return mk(EFFECTS_APPLY_INVALID, EFFECTS_HANDLE_NONE);
if (duration == 0 || (duration < 0 && duration != EFFECTS_DURATION_INFINITE))
        return mk(EFFECTS_APPLY_INVALID, EFFECTS_HANDLE_NONE);
if (!effects_curative_admits(im, kind))
        return mk(EFFECTS_APPLY_IMMUNE, EFFECTS_HANDLE_NONE);
const effects_def *d = effects_def_get(kind);
if (amplifier < 0) amplifier = 0;
if (amplifier > EFFECTS_MAX_AMPLIFIER) amplifier = EFFECTS_MAX_AMPLIFIER;
effects_instance *existing = effects_set_find(s, kind);
if (!slot) return mk(EFFECTS_APPLY_INVALID, EFFECTS_HANDLE_NONE);
effects_instance_set(slot, kind, amplifier, duration, source_id, ambient);
return mk(EFFECTS_APPLY_ADDED, effects_set_handle_of(s, slot));
}
