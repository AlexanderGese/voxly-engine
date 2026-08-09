#include "crafting_tag.h"
#include "crafting_book.h"
#include <string.h>

#define CRAFT_TAG_CAP 256

static unsigned s_tag[CRAFT_TAG_CAP];

void craft_tag_init(void) {
    memset(s_tag, 0, sizeof s_tag);
}

void craft_tag_set(int recipe_id, unsigned mask) {
    if (recipe_id < 0 || recipe_id >= CRAFT_TAG_CAP) return;
    s_tag[recipe_id] = mask;
}

void craft_tag_add(int recipe_id, craft_tag t) {
    if (recipe_id < 0 || recipe_id >= CRAFT_TAG_CAP) return;
    s_tag[recipe_id] |= (unsigned)t;
}

unsigned craft_tag_infer(block_id result) {
    switch (result) {
        case BLOCK_PLANKS:
        case BLOCK_COBBLE:
        case BLOCK_BRICK:
        case BLOCK_GLASS:
        case BLOCK_STONE:
            return CRAFT_TAG_BUILDING;
        case BLOCK_TORCH:
            return CRAFT_TAG_LIGHT | CRAFT_TAG_DECO;
        case BLOCK_SNOW:
        case BLOCK_ICE:
            return CRAFT_TAG_DECO;
        default:
            return CRAFT_TAG_MISC;
    }
}

unsigned craft_tag_get(int recipe_id) {
    if (recipe_id < 0 || recipe_id >= CRAFT_TAG_CAP) return CRAFT_TAG_NONE;
    if (s_tag[recipe_id] != CRAFT_TAG_NONE) return s_tag[recipe_id];
    // lazy fallback: infer from the result and cache it so the next read is
    // free. registration usually beats us to it, but tests poke recipes raw.
    const craft_recipe *r = craft_book_get(recipe_id);
    if (!r) return CRAFT_TAG_NONE;
    unsigned inferred = craft_tag_infer(r->result.id);
    s_tag[recipe_id] = inferred;
    return inferred;
}

int craft_tag_has(int recipe_id, craft_tag t) {
    return (craft_tag_get(recipe_id) & (unsigned)t) != 0;
}

int craft_tag_filter(craft_tag t, int *out, int cap) {
    int n = 0, total = craft_book_count();
    for (int i = 0; i < total && n < cap; i++)
        if (craft_tag_has(i, t)) out[n++] = i;
    return n;
}

const char *craft_tag_name(craft_tag t) {
    switch (t) {
        case CRAFT_TAG_BUILDING: return "building";
        case CRAFT_TAG_TOOLS:    return "tools";
        case CRAFT_TAG_LIGHT:    return "light";
        case CRAFT_TAG_DECO:     return "deco";
        case CRAFT_TAG_MISC:     return "misc";
        case CRAFT_TAG_NONE:     return "all";
        default:                 return "?";
    }
}
