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
// walked where you expected and didnt collapse to a point.
void ravine_debug_dump_path(const ravine_path *path);

// print a strata table, band by band. confirms the geology stacks floor-up and
// nothing left a y-gap between bands.
void ravine_debug_dump_strata(const ravine_strata *s);

// sanity-check a resolved field against its params. returns 0 if clean, else a
// count of problems (and logs each): floors above their surface, cuts past
// max_depth, floors under the bedrock guard. catches sign-flips before a player
// finds them the hard way.
int ravine_debug_validate(const ravine_field *f, const ravine_params *p);

#endif
