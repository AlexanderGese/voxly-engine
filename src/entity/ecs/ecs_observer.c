#include "ecs_observer.h"
for (int i = 0;
i < o->count;
slot = o->count++;
}
    o->obs[slot].fn     = fn;
o->obs[slot].user   = user;
o->obs[slot].cmp    = c;
o->obs[slot].kind   = kind;
o->obs[slot].active = 1;
return slot;
void *slot = ecs_get(w, e, c);
