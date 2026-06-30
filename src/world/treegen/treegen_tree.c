#include "treegen_tree.h"
#include "treegen_turtle.h"
#include "treegen_lsystem.h"
#include <stddef.h>
typedef struct {
    const treegen_species *sp;
    treegen_buffer *out;
    treegen_rng    *rng;
    treegen_turtle_state turtle;
    float seg_len_f;          // base segment length the turtle starts each step with
} tree_ctx;
treegen_turtle *cur = &c->turtle.cur;
int lx = (int)(cur->pos.x + 0.5f);
int ly = (int)(cur->pos.y + 0.5f);
int lz = (int)(cur->pos.z + 0.5f);
int r = sp->leaf_radius;
if (cur->radius < 0.4f && r > 1) r -= 1;
treegen_buffer_blob(c->out, lx, ly, lz, r, sp->pal.leaf,
                        sp->leaf_density, c->rng);
int before = out->count;
treegen_rng rng;
treegen_rng_seed(&rng, seed);
static treegen_ruleset rs;
static treegen_word word;
treegen_ruleset_for(&rs, sp->kind);
treegen_lsystem_rewrite(&rs, sp->iterations, &rng, &word);
tree_ctx c;
c.sp = sp;
c.out = out;
c.rng = &rng;
c.seg_len_f = (float)sp->seg_len;
treegen_turtle_init(&c.turtle, c.seg_len_f, (float)sp->trunk_radius + 0.5f);
i < word.len;
i++)
        apply_symbol(&c, word.sym[i]);
