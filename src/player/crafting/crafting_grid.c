#include "crafting_grid.h"
g->cell[idx(x, y)] = s;
return 1;
craft_stack *c = &g->cell[idx(x, y)];
c->count++;
return 1;
}
    // occupied by a different item, refuse.
    return 0;
for (int i = 0;
i < CRAFT_GRID_CELLS;
i++)
        if (!craft_stack_empty(&g->cell[i])) n++;
return n;
