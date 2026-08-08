#ifndef PLAYER_CRAFTING_H
#define PLAYER_CRAFTING_H
// umbrella header + module lifecycle. include this from the player code and
// you get the whole crafting api. the .c just sequences init/teardown so
// callers dont have to remember the book-then-recipes ordering.
#include "crafting_types.h"
#include "crafting_grid.h"
#include "crafting_match.h"
#include "crafting_book.h"
#include "crafting_table.h"
void pcraft_init(void);
void crafting_shutdown(void);
int  crafting_on_pickup(block_id id);
#endif
