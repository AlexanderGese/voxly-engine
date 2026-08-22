#ifndef CRAFT_RECIPE_H
#define CRAFT_RECIPE_H

#include "../world/block.h"

// 3x3 grid recipes. pattern matches exact block ids, 0 = empty.

#define RECIPE_MAX 64
#define GRID_SIZE   3

typedef struct {
    block_id pattern[GRID_SIZE][GRID_SIZE];
    block_id result;
    int      count;
    const char *name;
} recipe;

int           recipes_load(void);
int           recipes_count(void);
const recipe *recipes_get(int i);
const recipe *recipes_match(const block_id grid[GRID_SIZE][GRID_SIZE]);

#endif
