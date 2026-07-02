#ifndef RENDER_RENDERER_H
#define RENDER_RENDERER_H

#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "../world/world.h"

// one global renderer..

typedef struct {
    glid     prog_block;
    glid     prog_skybox;
    glid     prog_hud;
    texture  atlas;
    mesh    *chunk_meshes;   // parallel to chunk list via pointer tag
    int      meshes_cap;
} renderer;

int  renderer_init(renderer *r, const char *atlas_path);
void renderer_shutdown(renderer *r);

void renderer_draw(renderer *r, world *w, const camera *cam);

#endif
