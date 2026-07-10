#include "lt_cache.h"
#include <stdlib.h>
uint64_t key;
void *val;
hm_iter_init(&it, &c->map);
c->entry_count = 0;
c->tri_total   = 0;
lt_cache_entry *e = hashmap_get(&c->map, key);
