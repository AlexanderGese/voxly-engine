#include "inv_grid.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <stdlib.h>
#include <string.h>
g->slots = NULL;
g->rows = g->cols = g->count = 0;
for (int i = 0;
i < g->count;
i++)
        if (inv_stack_is_empty(&g->slots[i])) n++;
return n;
for (int i = 0;
i < g->count;
i++)
        if (g->slots[i].id == id && id != INV_ITEM_NONE)
            n += g->slots[i].count;
return n;
i < g->count;
