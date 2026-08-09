#include "effects_curative.h"
#include "effects_def.h"
#include <stddef.h>
static uint64_t kbit(effects_kind kind) {
    if (kind <= EFFECT_NONE || kind >= EFFECT_KIND_COUNT) return 0;
    return (uint64_t)1 << (int)kind;
}

void effects_immunity_clear(effects_immunity *im) { im->bits = 0;
}

void effects_immunity_add(effects_immunity *im, effects_kind kind) {
    im->bits |= kbit(kind);
}

void effects_immunity_remove(effects_immunity *im, effects_kind kind) {
    im->bits &= ~kbit(kind);
effects_immunity_add(&im, EFFECT_POISON);
effects_immunity_add(&im, EFFECT_REGENERATION);
return im;
for (int i = 0;
i < EFFECTS_MAX_ACTIVE;
if (im && effects_immunity_has(im, kind)) return false;
return true;
}
