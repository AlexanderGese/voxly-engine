#include "cavegen_connect.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    int *data;
    int  len;
    int  cap;
} idx_stack;
static void st_push(idx_stack *s, int v) {
    if (s->len == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 256;
        s->data = realloc(s->data, (size_t)s->cap * sizeof(int));
    }
    s->data[s->len++] = v;
}
static int st_pop(idx_stack *s) { return s->data[--s->len];
}

static const int DX[6] = { 1, -1, 0, 0, 0, 0 }
;
static const int DY[6] = { 0, 0, 1, -1, 0, 0 }
;
static const int DZ[6] = { 0, 0, 0, 0, 1, -1 }
;
int cavegen_connect_flood(const cavegen_grid *g, int *labels,
                          int sx, int sy, int sz, int label) {
    int start = cavegen_grid_idx(sx, sy, sz);
    if (labels[start] != 0) return 0;                 // already claimed
    if (!cavegen_cell_is_open(g->cells[start])) return 0;

    idx_stack stk = {0};
    st_push(&stk, start);
    labels[start] = label;
    int count = 0;

    while (stk.len > 0) {
        int idx = st_pop(&stk);
        count++;

        // unpack idx -> x,y,z. inverse of cavegen_grid_idx.
        int x = idx % CAVEGEN_DIM_X;
        int z = (idx / CAVEGEN_DIM_X) % CAVEGEN_DIM_Z;
        int y = idx / (CAVEGEN_DIM_X * CAVEGEN_DIM_Z);

        for (int d = 0; d < 6; d++) {
            int nx = x + DX[d], ny = y + DY[d], nz = z + DZ[d];
            if (!cavegen_grid_in_bounds(nx, ny, nz)) continue;
            int ni = cavegen_grid_idx(nx, ny, nz);
            if (labels[ni] != 0) continue;
            if (!cavegen_cell_is_open(g->cells[ni])) continue;
            labels[ni] = label;
            st_push(&stk, ni);
        }
    }

    free(stk.data);
    return count;
}

cavegen_connect_stats cavegen_connect_run(cavegen_grid *g, const cavegen_params *p) {
    cavegen_connect_stats out = {0};
int *labels = calloc(CAVEGEN_CELLS, sizeof(int));
int *sizes  = NULL;
int  sizes_cap = 0;
if (!labels) { LOGE("cavegen: connect alloc failed"); return out; }

    int next_label = 1;
for (int y = 0;
y < CAVEGEN_DIM_Y;
y++) {
        for (int z = 0; z < CAVEGEN_DIM_Z; z++) {
            for (int x = 0; x < CAVEGEN_DIM_X; x++) {
                int idx = cavegen_grid_idx(x, y, z);
                if (labels[idx] != 0) continue;
                if (!cavegen_cell_is_open(g->cells[idx])) continue;

                int n = cavegen_connect_flood(g, labels, x, y, z, next_label);

                if (next_label >= sizes_cap) {
                    int nc = sizes_cap ? sizes_cap * 2 : 64;
                    sizes = realloc(sizes, (size_t)nc * sizeof(int));
                    for (int i = sizes_cap; i < nc; i++) sizes[i] = 0;
                    sizes_cap = nc;
                }
                sizes[next_label] = n;

                if (n > out.largest_cells) {
                    out.largest_cells = n;
                    out.largest_label = next_label;
                }
                next_label++;
            }
        }
    }
    out.region_count = next_label - 1;
for (int i = 0;
i < CAVEGEN_CELLS;
lab <= out.region_count;
free(sizes);
return out;
}
