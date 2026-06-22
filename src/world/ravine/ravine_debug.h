#ifndef WORLD_RAVINE_DEBUG_H
#define WORLD_RAVINE_DEBUG_H
#include "ravine_field.h"
#include "ravine_path.h"
#include "ravine_strata.h"
#include "ravine_types.h"
// dev-only helpers. none of this runs in a real worldgen pass. its what i reach
// for when a ravine comes out as a dirt pillar or a bottomless slot and i need
// to see the resolved field instead of guessing from inside the game.
// dump the field's kind plane to stderr as ascii, one char per cell. '.'=outside
// '#'=wall, '_'=floor, ':'=rim. the ravine's shape should be obvious — a
// meandering ribbon of '_' walled by '#'.
void ravine_debug_print_kinds(const ravine_field *f);
// dump the cut-depth plane as a coarse ramp ('.' shallow .. '@' deep).
void ravine_debug_print_depth(const ravine_field *f);
// list a path's knots (index, xz, half_width, floor). for checking the spline
void ravine_debug_dump_path(const ravine_path *path);
void ravine_debug_dump_strata(const ravine_strata *s);
int ravine_debug_validate(const ravine_field *f, const ravine_params *p);
#endif
