#include "builder.h"

#include <string.h>

instancing_builder instancing_builder_new(int mesh_id) {
    instancing_builder b;
    memset(&b, 0, sizeof b);
    b.out.mesh_id = mesh_id;
    b.out.position = VEC3_ZERO;
    b.out.scale = vec3_new(1.0f, 1.0f, 1.0f);
    b.out.euler = VEC3_ZERO;
    b.out.tint[0] = b.out.tint[1] = b.out.tint[2] = b.out.tint[3] = 1.0f;
    b.out.light = 1.0f;
    // local_box left zero -> instancing_add borrows the mesh box.
    b.out.flags = INSTANCING_FLAG_ACTIVE;
    return b;
}

instancing_builder instancing_build_at(instancing_builder b, vec3 pos) {
    b.out.position = pos;
    return b;
}

instancing_builder instancing_build_scale(instancing_builder b, vec3 scale) {
    b.out.scale = scale;
    return b;
}

instancing_builder instancing_build_uniform(instancing_builder b, float s) {
    b.out.scale = vec3_new(s, s, s);
    return b;
}

instancing_builder instancing_build_euler(instancing_builder b, vec3 euler) {
    b.out.euler = euler;
    return b;
}

instancing_builder instancing_build_yaw(instancing_builder b, float radians) {
    // yaw is rotation about world up, which is our y euler component.
    b.out.euler.y = radians;
    return b;
}

instancing_builder instancing_build_tint(instancing_builder b,
                                         float r, float g, float bl, float a) {
    b.out.tint[0] = r;
    b.out.tint[1] = g;
    b.out.tint[2] = bl;
    b.out.tint[3] = a;
    return b;
}

instancing_builder instancing_build_light(instancing_builder b, float light) {
    // clamp to [0,1] so a stray value doesn't blow out the shader's modulate.
    if (light < 0.0f) light = 0.0f;
    if (light > 1.0f) light = 1.0f;
    b.out.light = light;
    return b;
}

instancing_builder instancing_build_box(instancing_builder b, aabb box) {
    b.out.local_box = box;
    return b;
}

instancing_builder instancing_build_no_cull(instancing_builder b) {
    b.out.flags |= INSTANCING_FLAG_NO_CULL;
    return b;
}

instancing_instance instancing_build_finish(instancing_builder b) {
    return b.out;
}
