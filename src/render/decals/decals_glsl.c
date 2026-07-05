#include "decals_glsl.h"
const char *DECALS_GLSL_VERT =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;
\n"          // unit cube corner, [-0.5,0.5]
    "layout(location = 1) in mat4 a_model;
\n"        // 1..4: cube->world
    "layout(location = 5) in mat4 a_inv_model;
\n"    // 5..8: world->cube
    "layout(location = 9) in vec4 a_uv_rect;
\n"      // xy = uv0, zw = uv1
    "layout(location = 10) in vec4 a_params;
\n"      // alpha, angle_fade, rough, flags
    "uniform mat4 u_view_proj;
\n"
    "out mat4  v_inv_model;
\n"
    "out vec4  v_uv_rect;
\n"
    "out vec4  v_params;
const char *DECALS_GLSL_FRAG =
    "#version 330 core\n"
    "in mat4  v_inv_model;
\n"
    "in vec4  v_uv_rect;
\n"
    "in vec4  v_params;
\n"
    // we render into the g-buffer's albedo + normal + material attachments with
    // blending on, so three outputs matching the mrt order.
    "layout(location = 0) out vec4 o_albedo;
\n"
    "layout(location = 1) out vec4 o_normal;
\n"
    "layout(location = 2) out vec4 o_material;
\n"
    "uniform sampler2D u_depth;
\n"      // g-buffer depth
    "uniform sampler2D u_gnormal;
\n"    // g-buffer normal, for the angle test
    "uniform sampler2D u_atlas;
\n"      // decal color (rgb) + coverage (a)
    "uniform mat4  u_inv_view_proj;
\n"  // clip -> world
    "uniform vec2  u_screen;
\n"         // viewport size in pixels
    "uniform vec3  u_tint;
\n"
    "uniform float u_normal_strength;
\n"
    "    float depth = texture(u_depth, suv).r;
\n"
    // sky / unwritten depth: nothing to project onto.
    "    if (depth >= 1.0) discard;
\n"
    "    vec3 wpos = reconstruct_world(suv, depth);
\n"
    // fold into the projector cube. outside [-0.5,0.5] on any axis => not us.
    "    vec3 local = (v_inv_model * vec4(wpos, 1.0)).xyz;
\n"
    "    if (any(greaterThan(abs(local), vec3(0.5)))) discard;
\n"
    // surface normal from the g-buffer (stored 0..1, decode to -1..1).\n"
    "    vec3 gn = texture(u_gnormal, suv).xyz * 2.0 - 1.0;
\n"
    // projector forward is local +z;
angle test against the surface normal so
    // we dont smear across walls facing away from the projection axis.\n"
    "    vec3 fwd = normalize((v_inv_model * vec4(0,0,1,0)).xyz);
\n"
    "    float facing = dot(normalize(gn), -fwd);
\n"
    "    float ang = v_params.y;
\n"
    "    if (facing < ang) discard;
\n"
    // edge feather: fade alpha toward the cube faces so the stamp has no hard
    // rectangular border.\n"
    "    vec3 e = (vec3(0.5) - abs(local)) * 2.0;
