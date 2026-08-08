#include "crafting_recipes.h"
#include "crafting_builder.h"
#include "crafting_book.h"
#include "../../world/block.h"
#include "../../world/block_ext.h"
#include "../../util/log.h"
#define _ BLOCK_AIR
#define W BLOCK_WOOD
#define P BLOCK_PLANKS
#define C BLOCK_COBBLE
#define S BLOCK_STONE
add(craft_build_shapeless("planks", ings, 1, BLOCK_PLANKS, 4));
}

    // 2 planks stacked -> sticks. we dont have a stick block so reuse fence
    // as the "stick-like" output;
add(craft_build_shaped("crafting_table", 2, 2, pat,
                               BLOCK_CRAFTING_TABLE, 1));
add(craft_build_3x3("furnace", pat, BLOCK_FURNACE, 1));
add(craft_build_shaped("wood_slab", 3, 1, pat, BLOCK_SLAB_WOOD, 6));
;
add(craft_build_3x3("ladder", pat, BLOCK_LADDER, 3));
;
add(craft_build_3x3("bookshelf", pat, BLOCK_BOOKSHELF, 1));
add(craft_build_shapeless("bricks", ings, 4, BLOCK_BRICK, 1));
}

    LOGI("crafting: registered %d recipes", craft_book_count());
