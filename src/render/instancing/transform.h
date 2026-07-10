#ifndef RENDER_INSTANCING_TRANSFORM_BAKE_H
#define RENDER_INSTANCING_TRANSFORM_BAKE_H

#include "instancing_types.h"

// turns an instance's cpu transform (pos/scale/euler) into the column-major
// model matrix the gpu record wants, and into a world-space aabb for culling.
// kept separate from the rest so the bake step is easy to test in isolation.

// build model = translate * rotz * roty * rotx * scale. order matters:
// scale first (object space), then rotate, then translate into the world.
mat4 instancing_xform_model(const instancing_instance *it);

// bake the model matrix straight into the lean gpu record, copying tint and
// light across too. does not touch flags.
void instancing_xform_bake(const instancing_instance *it,
                               instancing_gpu_instance *out);

// transform the instance's local box by its model matrix and return the
// tight world-space aabb that encloses the result. used by the frustum pass.
aabb instancing_xform_world_box(const instancing_instance *it);

#endif
