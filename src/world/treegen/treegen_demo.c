#include "treegen.h"
#include <stdio.h>
#include <string.h>

// offline self-test / ascii previewer for the vegetation stack. not built into
// the engine; compile this TU on its own (define TREEGEN_DEMO_MAIN) to eyeball a
// tree's silhouette without launching the game. mostly i used this to tune the
// l-system angles until oaks stopped looking like umbrellas.

int treegen_grow_one(treegen_buffer *out, treegen_kind kind, uint32_t seed) {
    if (kind == TREEGEN_BUSH) return treegen_bush_grow(out, seed);
    return treegen_tree_grow(out, kind, seed);
}

#ifdef TREEGEN_DEMO_MAIN

// flatten a grown buffer onto the xy plane (project along z) and print it, so a
// terminal shows the front elevation of the plant. '#' wood, '*' leaves.
static void preview_xy(const treegen_buffer *b) {
    if (b->count == 0) { printf("(empty)\n"); return; }
    int w = b->max_x - b->min_x + 1;
    int h = b->max_y - b->min_y + 1;
    if (w <= 0 || h <= 0 || w > 200 || h > 200) { printf("(too big)\n"); return; }

    char grid[200][201];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) grid[y][x] = ' ';
        grid[y][w] = '\0';
    }
    for (int i = 0; i < b->count; i++) {
        const treegen_voxel *v = &b->items[i];
        int gx = v->x - b->min_x;
        int gy = v->y - b->min_y;
        char ch = (v->id == BLOCK_LEAVES) ? '*' : '#';
        // leaves never hide an existing trunk char.
        if (grid[gy][gx] == '#' && ch == '*') continue;
        grid[gy][gx] = ch;
    }
    for (int y = h - 1; y >= 0; y--) printf("  %s\n", grid[y]);
}

static void run_one(const char *name, treegen_kind kind, uint32_t seed) {
    treegen_buffer b;
    treegen_buffer_init(&b);
    int n = treegen_grow_one(&b, kind, seed);
    printf("== %s  (seed=%u, %d voxels, bounds y %d..%d) ==\n",
           name, seed, n, b.min_y, b.max_y);
    preview_xy(&b);
    printf("\n");
    treegen_buffer_free(&b);
}

// quick distribution sanity: grow a kind across many seeds, make sure voxel
// counts stay in a believable band and nothing explodes the buffer.
static void stats(treegen_kind kind, const char *name) {
    int lo = 1 << 30, hi = 0;
    long sum = 0;
    const int N = 256;
    for (int i = 0; i < N; i++) {
        treegen_buffer b;
        treegen_buffer_init(&b);
        int n = treegen_grow_one(&b, kind, treegen_seed_mix(0xabcd, (uint32_t)i));
        if (n < lo) lo = n;
        if (n > hi) hi = n;
        sum += n;
        treegen_buffer_free(&b);
    }
    printf("%-7s over %d seeds: min=%d max=%d avg=%ld\n",
           name, N, lo, hi, sum / N);
}

int main(void) {
    run_one("oak",   TREEGEN_OAK,   12345u);
    run_one("pine",  TREEGEN_PINE,  777u);
    run_one("birch", TREEGEN_BIRCH, 99u);
    run_one("palm",  TREEGEN_PALM,  4242u);
    run_one("bush",  TREEGEN_BUSH,  31337u);

    printf("---- distribution ----\n");
    stats(TREEGEN_OAK,   "oak");
    stats(TREEGEN_PINE,  "pine");
    stats(TREEGEN_BIRCH, "birch");
    stats(TREEGEN_PALM,  "palm");
    return 0;
}

#endif
