#include "decals_glsl.h"

// the cube is drawn in world space: per-instance we supply `model` (decal-local
// cube -> world) so the rasteriser covers exactly the projector volume, plus
// `inv_model` (world -> decal-local) so the fragment stage can fold the surface
// point back into the [-0.5,0.5] cube cheaply. uv_rect / params ride alongside.
//
// the instance attribute layout (locations 1..9) must match decals_pass.c when
// it sets up the vertex array. break one, break both.
const char *DECALS_GLSL_VERT =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"          // unit cube corner, [-0.5,0.5]
    "layout(location = 1) in mat4 a_model;\n"        // 1..4: cube->world
    "layout(location = 5) in mat4 a_inv_model;\n"    // 5..8: world->cube
    "layout(location = 9) in vec4 a_uv_rect;\n"      // xy = uv0, zw = uv1
    "layout(location = 10) in vec4 a_params;\n"      // alpha, angle_fade, rough, flags
    "uniform mat4 u_view_proj;\n"
    "out mat4  v_inv_model;\n"
    "out vec4  v_uv_rect;\n"
    "out vec4  v_params;\n"
    "void main() {\n"
    "    vec4 world = a_model * vec4(a_pos, 1.0);\n"
    "    v_inv_model = a_inv_model;\n"
    "    v_uv_rect   = a_uv_rect;\n"
    "    v_params    = a_params;\n"
    "    gl_Position = u_view_proj * world;\n"
    "}\n";

// flags bit layout mirrors decals_types.h enum, packed into a float via the
// instance fill. we only need NORMAL_MAP, ADDITIVE and PROJECT_BACK here.
const char *DECALS_GLSL_FRAG =
    "#version 330 core\n"
    "in mat4  v_inv_model;\n"
    "in vec4  v_uv_rect;\n"
    "in vec4  v_params;\n"
    // we render into the g-buffer's albedo + normal + material attachments with
    // blending on, so three outputs matching the mrt order.
    "layout(location = 0) out vec4 o_albedo;\n"
    "layout(location = 1) out vec4 o_normal;\n"
    "layout(location = 2) out vec4 o_material;\n"
    "uniform sampler2D u_depth;\n"      // g-buffer depth
    "uniform sampler2D u_gnormal;\n"    // g-buffer normal, for the angle test
    "uniform sampler2D u_atlas;\n"      // decal color (rgb) + coverage (a)
    "uniform mat4  u_inv_view_proj;\n"  // clip -> world
    "uniform vec2  u_screen;\n"         // viewport size in pixels
    "uniform vec3  u_tint;\n"
    "uniform float u_normal_strength;\n"
    "\n"
    "vec3 reconstruct_world(vec2 uv, float depth) {\n"
    "    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);\n"
    "    vec4 w = u_inv_view_proj * ndc;\n"
    "    return w.xyz / w.w;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 suv = gl_FragCoord.xy / u_screen;\n"
    "    float depth = texture(u_depth, suv).r;\n"
    // sky / unwritten depth: nothing to project onto.
    "    if (depth >= 1.0) discard;\n"
    "    vec3 wpos = reconstruct_world(suv, depth);\n"
    // fold into the projector cube. outside [-0.5,0.5] on any axis => not us.
    "    vec3 local = (v_inv_model * vec4(wpos, 1.0)).xyz;\n"
    "    if (any(greaterThan(abs(local), vec3(0.5)))) discard;\n"
    // surface normal from the g-buffer (stored 0..1, decode to -1..1).\n"
    "    vec3 gn = texture(u_gnormal, suv).xyz * 2.0 - 1.0;\n"
    // projector forward is local +z; angle test against the surface normal so
    // we dont smear across walls facing away from the projection axis.\n"
    "    vec3 fwd = normalize((v_inv_model * vec4(0,0,1,0)).xyz);\n"
    "    float facing = dot(normalize(gn), -fwd);\n"
    "    float ang = v_params.y;\n"
    "    if (facing < ang) discard;\n"
    // edge feather: fade alpha toward the cube faces so the stamp has no hard
    // rectangular border.\n"
    "    vec3 e = (vec3(0.5) - abs(local)) * 2.0;\n"
    "    float edge = clamp(min(e.x, min(e.y, e.z)) * 4.0, 0.0, 1.0);\n"
    // angle feather: ease in across the cone instead of a hard cut.\n"
    "    float angle_w = smoothstep(ang, min(ang + 0.25, 1.0), facing);\n"
    "    vec2 uv = mix(v_uv_rect.xy, v_uv_rect.zw, local.xy + 0.5);\n"
    "    vec4 tex = texture(u_atlas, uv);\n"
    "    float a = tex.a * v_params.x * edge * angle_w;\n"
    "    if (a <= 0.003) discard;\n"
    "    o_albedo = vec4(tex.rgb * u_tint, a);\n"
    // normal blend: perturb toward +z of the projector, weighted by strength
    // and the resolved alpha. material.g carries roughness from params.z.\n"
    "    vec3 dn = mix(gn, fwd, u_normal_strength * a);\n"
    "    o_normal = vec4(normalize(dn) * 0.5 + 0.5, a);\n"
    "    o_material = vec4(0.0, v_params.z, 0.0, a);\n"
    "}\n";
