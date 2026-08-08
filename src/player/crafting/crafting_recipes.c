#include "crafting_recipes.h"
#include "crafting_builder.h"
#include "crafting_book.h"
#include "../../world/block.h"
#include "../../world/block_ext.h"
#include "../../util/log.h"

// the vanilla recipe set. shorthands keep the pattern tables readable.
#define _ BLOCK_AIR
#define W BLOCK_WOOD
#define P BLOCK_PLANKS
#define C BLOCK_COBBLE
#define S BLOCK_STONE

static void add(craft_recipe r) {
    craft_book_add(&r);
}

void craft_recipes_register_all(void) {
    // log -> 4 planks. shapeless, single ingredient.
    {
        block_id ings[] = { W };
        add(craft_build_shapeless("planks", ings, 1, BLOCK_PLANKS, 4));
    }

    // 2 planks stacked -> sticks. we dont have a stick block so reuse fence
    // as the "stick-like" output; the engine treats it as a thin item.
    {
        block_id pat[] = { P, P };   // 1x2 column
        add(craft_build_shaped("sticks", 1, 2, pat, BLOCK_FENCE, 4));
    }

    // crafting table: 2x2 of planks.
    {
        block_id pat[] = { P, P,
                           P, P };
        add(craft_build_shaped("crafting_table", 2, 2, pat,
                               BLOCK_CRAFTING_TABLE, 1));
    }

    // chest: ring of planks around an empty middle.
    {
        block_id pat[] = { P, P, P,
                           P, _, P,
                           P, P, P };
        add(craft_build_3x3("chest", pat, BLOCK_CHEST_BLOCK, 1));
    }

    // furnace: ring of cobble.
    {
        block_id pat[] = { C, C, C,
                           C, _, C,
                           C, C, C };
        add(craft_build_3x3("furnace", pat, BLOCK_FURNACE, 1));
    }

    // stone slab: 3 stone in a row.
    {
        block_id pat[] = { S, S, S };
        add(craft_build_shaped("stone_slab", 3, 1, pat, BLOCK_SLAB_STONE, 6));
    }

    // wood slab: 3 planks in a row.
    {
        block_id pat[] = { P, P, P };
        add(craft_build_shaped("wood_slab", 3, 1, pat, BLOCK_SLAB_WOOD, 6));
    }

    // fence run: planks-stick-planks twice (mirror-friendly).
    {
        block_id pat[] = { P, BLOCK_FENCE, P,
                           P, BLOCK_FENCE, P };
        add(craft_build_shaped("fence", 3, 2, pat, BLOCK_FENCE, 3));
    }

    // ladder: two stick rails with a rung.
    {
        block_id F = BLOCK_FENCE;
        block_id pat[] = { F, _, F,
                           F, F, F,
                           F, _, F };
        add(craft_build_3x3("ladder", pat, BLOCK_LADDER, 3));
    }

    // torch: coal on a stick.
    {
        block_id pat[] = { BLOCK_COAL_ORE,
                           BLOCK_FENCE };
        add(craft_build_shaped("torch", 1, 2, pat, BLOCK_TORCH, 4));
    }

    // bookshelf: planks rows around books (no book item, use wool as filler).
    {
        block_id B = BLOCK_WOOL_WHITE;
        block_id pat[] = { P, P, P,
                           B, B, B,
                           P, P, P };
        add(craft_build_3x3("bookshelf", pat, BLOCK_BOOKSHELF, 1));
    }

    // tnt: gravel + cobble checker. silly but exercises a full grid.
    {
        block_id G = BLOCK_GRAVEL;
        block_id pat[] = { G, C, G,
                           C, G, C,
                           G, C, G };
        add(craft_build_3x3("tnt", pat, BLOCK_TNT, 1));
    }

    // brick block from 4 clay, shapeless.
    {
        block_id ings[] = { BLOCK_CLAY, BLOCK_CLAY,
                            BLOCK_CLAY, BLOCK_CLAY };
        add(craft_build_shapeless("bricks", ings, 4, BLOCK_BRICK, 1));
    }

    LOGI("crafting: registered %d recipes", craft_book_count());
}

#undef _
#undef W
#undef P
#undef C
#undef S
