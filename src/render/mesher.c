#include "mesher.h"
#include "../world/block.h"
#include "../config.h"

#include <stdlib.h>
#include <string.h>

// face order: +x, -x, +y, -y, +z, -z
// each face = 6 vertices (2 triangles)

static const float face_verts[6][6][3] = {
    // +x
    {{1,0,0},{1,1,0},{1,1,1},{1,0,0},{1,1,1},{1,0,1}},
    // -x
    {{0,0,1},{0,1,1},{0,1,0},{0,0,1},{0,1,0},{0,0,0}},
    // +y
    {{0,1,0},{0,1,1},{1,1,1},{0,1,0},{1,1,1},{1,1,0}},
    // -y
    {{0,0,1},{0,0,0},{1,0,0},{0,0,1},{1,0,0},{1,0,1}},
    // +z
    {{1,0,1},{1,1,1},{0,1,1},{1,0,1},{0,1,1},{0,0,1}},
    // -z
    {{0,0,0},{0,1,0},{1,1,0},{0,0,0},{1,1,0},{1,0,0}}
};

static const float face_uvs[6][6][2] = {
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
    {{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}},
};

static void neighbor_offset(int face, int *dx, int *dy, int *dz) {
    static const int d[6][3] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    };
    *dx = d[face][0];
    *dy = d[face][1];
    *dz = d[face][2];
}

static float compute_light(world *w, int wx, int wy, int wz) {
    int s = world_get_sunlight(w, wx, wy, wz);
    int b = world_get_blocklight(w, wx, wy, wz);
    int max = s > b ? s : b;
    if (max > MAX_LIGHT) max = MAX_LIGHT;
    if (max < 3) max = 3;
    return (float)max / (float)MAX_LIGHT;
}

void mesher_build_chunk(world *w, chunk *c) {
    int base_x = c->cx * CHUNK_SIZE_X;
    int base_z = c->cz * CHUNK_SIZE_Z;

    // build vertex data into a temp buffer
    int cap = 4096;
    int count = 0;
    vertex *verts = malloc(cap * sizeof(vertex));

    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                block_id id = chunk_get_block(c, x, y, z);
                if (block_is_air(id)) continue;

                int wx = base_x + x;
                int wz = base_z + z;

                for (int face = 0; face < 6; face++) {
                    int dx, dy, dz;
                    neighbor_offset(face, &dx, &dy, &dz);
                    block_id n = world_get_block(w, wx + dx, y + dy, wz + dz);
                    if (block_is_opaque(n)) continue;
                    if (!block_is_opaque(id) && n == id) continue;

                    int tile = block_face_tile(id, face);
                    float tx = (float)(tile % ATLAS_TILES_X) / (float)ATLAS_TILES_X;
                    float ty = (float)(tile / ATLAS_TILES_X) / (float)ATLAS_TILES_Y;
                    float ts = 1.0f / (float)ATLAS_TILES_X;

                    float l = compute_light(w, wx + dx, y + dy, wz + dz);

                    if (count + 6 > cap) {
                        cap *= 2;
                        verts = realloc(verts, cap * sizeof(vertex));
                    }
                    for (int v = 0; v < 6; v++) {
                        vertex *vx = &verts[count++];
                        vx->x = (float)base_x + (float)x + face_verts[face][v][0];
                        vx->y = (float)y + face_verts[face][v][1];
                        vx->z = (float)base_z + (float)z + face_verts[face][v][2];
                        vx->u = tx + face_uvs[face][v][0] * ts;
                        vx->v = ty + face_uvs[face][v][1] * ts;
                        vx->light = l;
                    }
                }
            }
        }
    }

    // create GL resources on first use
    if (c->vao == 0) {
        glGenVertexArrays(1, &c->vao);
        glGenBuffers(1, &c->vbo);
        glBindVertexArray(c->vao);
        glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              (void*)(5 * sizeof(float)));
        glBindVertexArray(0);
    }

    // upload
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(vertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    c->vertex_count = count;
    c->dirty = 0;

    free(verts);
}
