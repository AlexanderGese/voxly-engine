#include "voxel_mask.h"
#include <string.h>

void mb_mask_reset(mb_mask *m, int w, int h) {
    m->w = w;
    m->h = h;
    // only clear the part we use. the backing array is sized for the worst
    // case so a full memset would touch a lot of cold memory for an x/z slice.
    memset(m->cells, 0, (size_t)w * h * sizeof(mb_cell));
}

int mb_cell_eq(const mb_cell *a, const mb_cell *b) {
    if (!a->present || !b->present) return 0;
    if (a->block != b->block) return 0;
    if (a->tile  != b->tile)  return 0;

    // shading has to match per corner or the merge would flatten gradients.
    for (int i = 0; i < 4; i++) {
        if (a->ao[i]    != b->ao[i])    return 0;
        if (a->light[i] != b->light[i]) return 0;
    }
    return 1;
}
