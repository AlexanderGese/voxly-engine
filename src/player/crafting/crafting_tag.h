#ifndef PLAYER_CRAFTING_TAG_H
#define PLAYER_CRAFTING_TAG_H

#include "crafting_types.h"

// recipe categories for the book tabs. each recipe gets a tag bitmask (a recipe
// can sit under more than one tab, e.g. a torch is both "light" and "deco").
// tags are assigned once at registration; if a recipe never gets a tag we infer
// one from its result block so the data side can stay lazy.

typedef enum {
    CRAFT_TAG_NONE     = 0,
    CRAFT_TAG_BUILDING = 1 << 0,   // planks, brick, glass...
    CRAFT_TAG_TOOLS    = 1 << 1,
    CRAFT_TAG_LIGHT    = 1 << 2,   // torches, lamps
    CRAFT_TAG_DECO     = 1 << 3,
    CRAFT_TAG_MISC     = 1 << 4,
} craft_tag;

void craft_tag_init(void);

// set / get the tag mask for a recipe id. set replaces, _add OR's in.
void craft_tag_set(int recipe_id, unsigned mask);
void craft_tag_add(int recipe_id, craft_tag t);
unsigned craft_tag_get(int recipe_id);

// does recipe `id` carry tag `t`? if the recipe has no explicit tag this falls
// back to inferring from the result block.
int  craft_tag_has(int recipe_id, craft_tag t);

// infer a sensible tag from a result block id. used as the fallback and by the
// registration code so it doesnt have to tag every single recipe by hand.
unsigned craft_tag_infer(block_id result);

// fill `out` with recipe ids carrying tag `t`, scanning the whole book. returns
// count written (capped at `cap`).
int  craft_tag_filter(craft_tag t, int *out, int cap);

// human label for a single tag bit, for the tab strip. returns a static string,
// "?" for an unknown/combined mask.
const char *craft_tag_name(craft_tag t);

#endif
