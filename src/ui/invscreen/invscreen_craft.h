#ifndef UI_INVSCREEN_CRAFT_H
#define UI_INVSCREEN_CRAFT_H

// the crafting glue. sits between the 3x3 craft-in region of the model and the
// recipe matcher. every time the input changes we re-resolve the output cell and
// remember which recipe (if any) backs it. taking the output then consumes one
// of each input.
//
// the model owns the slots; this just reads the craft-in region into a flat
// block_id grid, asks the matcher, and writes the result into the craft-out
// cell. nothing here allocates.

#include "invscreen_model.h"
#include "invscreen_recipe.h"

typedef struct {
    const invscreen_recipe *recipe;  // current match, NULL if none
    int                     dirty;   // craft-in changed, needs a re-resolve
} invscreen_craft;

void invscreen_craft_init(invscreen_craft *cr);

// mark the craft state stale. call after any mutation to a craft-in slot. cheap,
// just sets a flag; the actual matching happens in resolve().
void invscreen_craft_touch(invscreen_craft *cr);

// re-run the matcher if dirty and stamp the output cell. safe to call every
// frame; it no-ops when nothing changed. returns 1 if the output changed.
int invscreen_craft_resolve(invscreen_craft *cr, invscreen_model *m);

// consume one of each non-empty craft-in slot. called when the player actually
// takes the output. returns 1 if a full set was consumed, 0 if the inputs no
// longer satisfy the recipe (in which case nothing is removed).
int invscreen_craft_consume(invscreen_craft *cr, invscreen_model *m);

// how many full batches the current inputs can produce without refilling. used
// for shift-click "craft all". 0 when there's no recipe.
int invscreen_craft_batches(const invscreen_craft *cr, const invscreen_model *m);

#endif
