#include "pass_split.h"
#include "meshbuild.h"
#include "../../util/darray.h"

mb_pass mb_pass_of(block_id id) {
    // opaque blocks are trivially the opaque pass. for the rest we treat any
    // non-opaque, non-air block as translucent — thats water/glass/ice today.
    // if leaves ever get true alpha cutout they can move here too.
    if (block_is_opaque(id)) return MB_PASS_OPAQUE;
    if (block_is_air(id))    return MB_PASS_OPAQUE;   // wont emit anyway
    return MB_PASS_TRANSLUCENT;
}

void mb_chunk_mesh_init(mb_chunk_mesh *m) {
    for (int p = 0; p < MB_PASS_COUNT; p++)
        meshbuild_result_init(&m->pass[p]);
}

void mb_chunk_mesh_free(mb_chunk_mesh *m) {
    for (int p = 0; p < MB_PASS_COUNT; p++)
        meshbuild_result_free(&m->pass[p]);
}

int mb_chunk_mesh_verts(const mb_chunk_mesh *m) {
    int total = 0;
    for (int p = 0; p < MB_PASS_COUNT; p++)
        total += (int)darr_len(m->pass[p].verts);
    return total;
}
