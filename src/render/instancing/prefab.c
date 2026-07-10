#include "prefab.h"

#include "../../config.h"
#include "../../util/darray.h"

// resolve an atlas tile id to its [u0,v0]-[u1,v1] rect. same scheme the mesher
// uses: tiles laid out left-to-right, top-to-bottom in an ATLAS_TILES_X grid.
static void tile_uv(int tile, float *u0, float *v0, float *u1, float *v1) {
    int tx = tile % ATLAS_TILES_X;
    int ty = tile / ATLAS_TILES_X;
    float sx = 1.0f / (float)ATLAS_TILES_X;
    float sy = 1.0f / (float)ATLAS_TILES_Y;
    *u0 = (float)tx * sx;
    *v0 = (float)ty * sy;
    *u1 = *u0 + sx;
    *v1 = *v0 + sy;
}

// push one vertex. tiny helper so the face builders read like geometry, not
// like a wall of struct literals.
static void push_v(instancing_base_vertex **out, float x, float y, float z,
                   float u, float v) {
    instancing_base_vertex vert = { x, y, z, u, v, 1.0f };
    darr_push(*out, vert);
}

// emit a quad as two triangles from four corners, winding a-b-c, a-c-d.
// uv corners track the vertex corners (a=00, b=10, c=11, d=01).
static void push_quad(instancing_base_vertex **out,
                      vec3 a, vec3 b, vec3 c, vec3 d,
                      float u0, float v0, float u1, float v1) {
    push_v(out, a.x, a.y, a.z, u0, v0);
    push_v(out, b.x, b.y, b.z, u1, v0);
    push_v(out, c.x, c.y, c.z, u1, v1);

    push_v(out, a.x, a.y, a.z, u0, v0);
    push_v(out, c.x, c.y, c.z, u1, v1);
    push_v(out, d.x, d.y, d.z, u0, v1);
}

int instancing_prefab_cube(instancing_base_vertex **out, int tile) {
    float u0, v0, u1, v1;
    tile_uv(tile, &u0, &v0, &u1, &v1);

    const float h = 0.5f;
    // eight corners of the unit cube.
    vec3 p000 = { -h, -h, -h }, p100 = { h, -h, -h };
    vec3 p110 = {  h,  h, -h }, p010 = { -h,  h, -h };
    vec3 p001 = { -h, -h,  h }, p101 = { h, -h,  h };
    vec3 p111 = {  h,  h,  h }, p011 = { -h,  h,  h };

    // six faces. winding chosen so front faces point outward (ccw from
    // outside). matches the engine's default GL_CCW front-face.
    push_quad(out, p001, p101, p111, p011, u0, v0, u1, v1); // +z front
    push_quad(out, p100, p000, p010, p110, u0, v0, u1, v1); // -z back
    push_quad(out, p101, p100, p110, p111, u0, v0, u1, v1); // +x right
    push_quad(out, p000, p001, p011, p010, u0, v0, u1, v1); // -x left
    push_quad(out, p011, p111, p110, p010, u0, v0, u1, v1); // +y top
    push_quad(out, p000, p100, p101, p001, u0, v0, u1, v1); // -y bottom

    return (int)darr_len(*out);
}

int instancing_prefab_cross(instancing_base_vertex **out, int tile) {
    float u0, v0, u1, v1;
    tile_uv(tile, &u0, &v0, &u1, &v1);

    const float h = 0.5f;
    // first vertical quad, diagonal from (-h,-h) to (+h,+h) in xz.
    vec3 a1 = { -h, 0.0f, -h }, b1 = {  h, 0.0f,  h };
    vec3 c1 = {  h, 1.0f,  h }, d1 = { -h, 1.0f, -h };
    push_quad(out, a1, b1, c1, d1, u0, v0, u1, v1);

    // second vertical quad, the other diagonal.
    vec3 a2 = { -h, 0.0f,  h }, b2 = {  h, 0.0f, -h };
    vec3 c2 = {  h, 1.0f, -h }, d2 = { -h, 1.0f,  h };
    push_quad(out, a2, b2, c2, d2, u0, v0, u1, v1);

    return (int)darr_len(*out);
}

int instancing_prefab_quad(instancing_base_vertex **out, int tile) {
    float u0, v0, u1, v1;
    tile_uv(tile, &u0, &v0, &u1, &v1);

    const float h = 0.5f;
    vec3 a = { -h, 0.0f, -h }, b = {  h, 0.0f, -h };
    vec3 c = {  h, 0.0f,  h }, d = { -h, 0.0f,  h };
    push_quad(out, a, b, c, d, u0, v0, u1, v1);

    return (int)darr_len(*out);
}

aabb instancing_prefab_cube_box(void) {
    return aabb_make(vec3_new(-0.5f, -0.5f, -0.5f),
                     vec3_new( 0.5f,  0.5f,  0.5f));
}

aabb instancing_prefab_cross_box(void) {
    return aabb_make(vec3_new(-0.5f, 0.0f, -0.5f),
                     vec3_new( 0.5f, 1.0f,  0.5f));
}

aabb instancing_prefab_quad_box(void) {
    // give the flat quad a sliver of vertical thickness so a degenerate box
    // doesn't confuse the frustum test.
    return aabb_make(vec3_new(-0.5f, -0.01f, -0.5f),
                     vec3_new( 0.5f,  0.01f,  0.5f));
}
