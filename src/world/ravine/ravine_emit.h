#ifndef WORLD_RAVINE_EMIT_H
#define WORLD_RAVINE_EMIT_H
#include "ravine_field.h"
#include "ravine_strata.h"
#include "../../util/darray.h"
// the edit-list flavour of the carve. ravine_carve writes straight into a chunk,
// which is what worldgen wants, but a few callers (the save-replay tooling, the
ravine_edit *ravine_emit_build(const ravine_field *f,
                               const ravine_strata *strata,
                               const ravine_params *p, size_t *out_count);
size_t ravine_emit_count(const ravine_field *f);
#endif
