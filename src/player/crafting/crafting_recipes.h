#ifndef PLAYER_CRAFTING_RECIPES_H
#define PLAYER_CRAFTING_RECIPES_H

// the actual recipe data. call once after craft_book_init() to populate the
// book with the vanilla set. kept separate from the book so the book stays a
// dumb container and the data lives in one obvious place.

void craft_recipes_register_all(void);

#endif
