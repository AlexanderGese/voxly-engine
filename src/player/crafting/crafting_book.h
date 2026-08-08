#ifndef PLAYER_CRAFTING_BOOK_H
#define PLAYER_CRAFTING_BOOK_H

#include "crafting_types.h"

// the recipe book: the global list of every known recipe, plus the per-player
// "unlocked" bitset that drives which recipes show up in the ui. recipes are
// registered at startup and never removed, so ids are stable indices.

void craft_book_init(void);
void craft_book_shutdown(void);

// register a recipe. takes a copy. returns the assigned recipe id (>=0), or
// -1 if the book is full. you normally dont call this directly; the builder
// in crafting_recipes.c does. but tests use it.
int  craft_book_add(const craft_recipe *r);

int  craft_book_count(void);
const craft_recipe *craft_book_get(int id);

// find the first registered recipe that the grid satisfies. returns its id or
// -1. this is the hot path the crafting table calls every time the grid
// changes.
int  craft_book_find(const craft_grid *g);

// like find but only considers recipes the player has unlocked. used when we
// want the result preview to respect the recipe-book gating.
int  craft_book_find_unlocked(const craft_grid *g);

// ---- unlock bitset ----
void craft_book_unlock(int recipe_id);
int  craft_book_is_unlocked(int recipe_id);
int  craft_book_unlocked_count(void);

// auto-unlock any recipe that uses `id` as an ingredient. called when the
// player first picks up a new block, so the book "discovers" recipes.
int  craft_book_discover_by_item(block_id id);

#endif
