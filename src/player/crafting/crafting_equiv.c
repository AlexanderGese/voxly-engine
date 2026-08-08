#include "crafting_equiv.h"
#include "crafting_grid.h"
#include <string.h>
static block_id s_canon[256];
static int      s_inited;
void craft_equiv_init(void) {
    if (s_inited) return;
    for (int i = 0; i < 256; i++) s_canon[i] = (block_id)i;

    // default groups. we don't have many block variants, so this is short:
    // cobble and stone craft-substitute for each other in the rough-building
    // recipes, and packed snow/ice read the same in the cold-deco recipes.
    craft_equiv_join(BLOCK_STONE, BLOCK_COBBLE);
    craft_equiv_join(BLOCK_SNOW,  BLOCK_ICE);

    s_inited = 1;
}

void craft_equiv_join(block_id canonical, block_id member) {
    if (member == BLOCK_AIR) return;
block_id old = s_canon[member];
s_canon[member] = canonical;
if (old != member) {
        for (int i = 0; i < 256; i++)
            if (s_canon[i] == old) s_canon[i] = canonical;
    }
}

block_id craft_equiv_canon(block_id id) {
    return s_canon[id];
}

int craft_equiv_same(block_id a, block_id b) {
    return s_canon[a] == s_canon[b];
}

int craft_equiv_normalize(struct craft_grid *gg) {
    craft_grid *g = (craft_grid *)gg;
int changed = 0;
for (int i = 0;
i < CRAFT_GRID_CELLS;
i++) {
        craft_stack *c = &g->cell[i];
        if (craft_stack_empty(c)) continue;
        block_id cn = s_canon[c->id];
        if (cn != c->id) { c->id = cn; changed++; }
    }
    return changed;
}

int craft_equiv_group_count(void) {
    // count canonical ids that have at least one extra member pointing at them.
    int members[256];
    memset(members, 0, sizeof members);
    for (int i = 0; i < 256; i++)
        members[s_canon[i]]++;
    int groups = 0;
    for (int i = 0; i < 256; i++)
        if (members[i] > 1) groups++;   // canonical + >=1 other member
    return groups;
}
