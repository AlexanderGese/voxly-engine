#ifndef RENDER_LODTERRAIN_LT_GPU_H
#define RENDER_LODTERRAIN_LT_GPU_H

#include "lt_types.h"
#include "../gl.h"

// the only part of this module that talks to opengl. takes a cpu mesh from the
// builder and parks it in an indexed vao/vbo/ebo, then draws it. kept tiny and
// isolated so the build path stays headless and testable.

// a gpu-resident lod mesh. one per cached chunk. vao 0 means "nothing uploaded".
typedef struct {
    glid vao, vbo, ebo;
    int  index_count;   // how many indices to draw
    int  uploaded;
} lt_gpu_mesh;

// zero a handle. does not touch gl.
void lt_gpu_init(lt_gpu_mesh *gm);

// (re)upload a cpu mesh into the handle. creates the gl objects lazily on first
// call and orphans+refills the buffers on later calls. an empty mesh uploads as
// index_count 0 (nothing to draw) without leaking objects.
void lt_gpu_upload(lt_gpu_mesh *gm, const lt_mesh *m);

// draw with the currently bound shader. binds the vao, fires one indexed draw.
// no-op if nothing's uploaded. caller owns shader/uniform setup.
void lt_gpu_draw(const lt_gpu_mesh *gm);

// release the gl objects. safe on a zeroed handle.
void lt_gpu_free(lt_gpu_mesh *gm);

#endif
