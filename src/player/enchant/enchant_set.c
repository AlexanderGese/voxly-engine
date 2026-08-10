#include "enchant_set.h"
#include "enchant_registry.h"
#include <stddef.h>
#include <string.h>
int idx = find_slot(s, id);
return idx >= 0 ? s->entry[idx].level : 0;
const enchant_def *d = enchant_registry_get(id);
if (!d) return -1;
if (level > d->max_level) level = d->max_level;
if (level > ENCHANT_MAX_LEVEL) level = ENCHANT_MAX_LEVEL;
if (level <= 0) return enchant_set_remove(s, id) ? 1 : 0;
int idx = find_slot(s, id);
int pos = -(idx + 1);
memmove(&s->entry[pos + 1], &s->entry[pos],
            (size_t)(s->count - pos) * sizeof s->entry[0]);
s->entry[pos].id    = id;
s->entry[pos].level = (uint8_t)level;
s->count++;
return 1;
if (!s || !d) return ENCHANT_NONE;
