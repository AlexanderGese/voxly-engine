#ifndef RENDER_INSTANCING_BUILDER_H
#define RENDER_INSTANCING_BUILDER_H

// little fluent-ish helper for filling an instancing_instance without writing
// the same six lines at every call site. you start from a default (identity
// scale, white tint, full light), then chain setters, then read .out. not a
// real builder pattern — c doesn't do methods — but the chaining reads okay
// because every setter returns the builder by value.
//
// instancing_instance it = instancing_build(
// instancing_build_tint(
// instancing_build_at(instancing_builder(mesh_id),
// vec3_new(10, 64, 8)),
// 1, 0.6f, 0.6f, 1)).out;
//
// yeah the nesting is ugly. most callers just grab a builder, poke a couple of
// fields and call _finish. the chaining is there if you want it.

#include "instancing_types.h"

typedef struct {
    instancing_instance out;
} instancing_builder;

// start a builder for `mesh_id` with sane defaults. local_box is left zero so
// instancing_add() will borrow the mesh's box; override with _box() if you
// want a custom cull volume.
instancing_builder instancing_builder_new(int mesh_id);

instancing_builder instancing_build_at(instancing_builder b, vec3 pos);
instancing_builder instancing_build_scale(instancing_builder b, vec3 scale);
instancing_builder instancing_build_uniform(instancing_builder b, float s);
instancing_builder instancing_build_euler(instancing_builder b, vec3 euler);
instancing_builder instancing_build_yaw(instancing_builder b, float radians);
instancing_builder instancing_build_tint(instancing_builder b,
                                         float r, float g, float bl, float a);
instancing_builder instancing_build_light(instancing_builder b, float light);
instancing_builder instancing_build_box(instancing_builder b, aabb box);
instancing_builder instancing_build_no_cull(instancing_builder b);

// pull the finished record out. equivalent to reading b.out directly, but
// reads nicer at the end of a chain.
instancing_instance instancing_build_finish(instancing_builder b);

#endif
