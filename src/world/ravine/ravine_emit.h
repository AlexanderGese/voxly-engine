#ifndef WORLD_RAVINE_EMIT_H
#define WORLD_RAVINE_EMIT_H
#include "ravine_field.h"
#include "ravine_strata.h"
#include "../../util/darray.h"
// the edit-list flavour of the carve. ravine_carve writes straight into a chunk,
// which is what worldgen wants, but a few callers (the save-replay tooling, the
// structure-overlap checker, and the tests) want the carve as a flat list of
// ravine_edit records they can inspect, diff or apply somewhere else. this pass
// produces exactly that: walk the field, emit one air edit per cleared cell and
// one strata edit per exposed wall face, in world coords. it never touches a
ravine_edit *ravine_emit_build(const ravine_field *f,
                               const ravine_strata *strata,
                               const ravine_params *p, size_t *out_count);
size_t ravine_emit_count(const ravine_field *f);
#endif
