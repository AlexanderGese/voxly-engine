#include "treegen_tree.h"
#include "treegen_turtle.h"
#include "treegen_lsystem.h"
#include <stddef.h>

// interpret a rewritten l-system word into voxels. the turtle does the geometry;
// here we map symbols to turtle ops and decide where leaf blobs land. angle
// jitter is rolled per-turn so no two limbs bend the same, but it's all driven
// by the plant rng so the result is still deterministic per seed.

typedef struct {
    const treegen_species *sp;
    treegen_buffer *out;
    treegen_rng    *rng;
    treegen_turtle_state turtle;
    float seg_len_f;          // base segment length the turtle starts each step with
} tree_ctx;

// shrink a turtle's length+radius one notch, used by '!'. clamps so a deep tree
// doesn't collapse a branch to nothing before it can sprout a leaf.
static void shrink(treegen_turtle_state *t) {
    t->cur.length *= 0.78f;
    t->cur.radius *= 0.66f;
    if (t->cur.length < 1.0f) t->cur.length = 1.0f;
    if (t->cur.radius < 0.0f) t->cur.radius = 0.0f;
}

// drop a leaf cluster at the current tip. radius eased down for thinner twigs so
// the canopy isn't a uniform wall of green.
static void leaf_here(tree_ctx *c) {
    const treegen_species *sp = c->sp;
    treegen_turtle *cur = &c->turtle.cur;
    int lx = (int)(cur->pos.x + 0.5f);
    int ly = (int)(cur->pos.y + 0.5f);
    int lz = (int)(cur->pos.z + 0.5f);

    int r = sp->leaf_radius;
    // thinner twigs (small radius left) get a smaller blob.
    if (cur->radius < 0.4f && r > 1) r -= 1;
    treegen_buffer_blob(c->out, lx, ly, lz, r, sp->pal.leaf,
                        sp->leaf_density, c->rng);
}

static void apply_symbol(tree_ctx *c, treegen_sym s) {
    const treegen_species *sp = c->sp;
    float jit = sp->angle_jitter;
    treegen_turtle_state *t = &c->turtle;

    switch (s) {
    case TG_SYM_FWD: {
        block_id wood = sp->pal.wood;
        // bail before scraping max_height so a wild branch can't tower forever.
        if (t->cur.pos.y > (float)sp->max_height) wood = BLOCK_AIR;
        treegen_turtle_forward(t, c->out, wood, NULL, NULL, NULL);
        break;
    }
    case TG_SYM_MOVE:
        treegen_turtle_forward(t, c->out, BLOCK_AIR, NULL, NULL, NULL);
        break;
    case TG_SYM_PUSH:
        treegen_turtle_push(t);
        break;
    case TG_SYM_POP:
        // popping just ends a branch and restores state. foliage is placed by
        // explicit 'L' symbols in the grammar, not here -- dropping a blob on
        // every pop too quadrupled the canopy and tanked the voxel count.
        treegen_turtle_pop(t);
        break;
    case TG_SYM_YAW_L:
        treegen_turtle_yaw(t,  sp->yaw_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_YAW_R:
        treegen_turtle_yaw(t, -sp->yaw_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_PITCH_D:
        treegen_turtle_pitch(t,  sp->pitch_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_PITCH_U:
        treegen_turtle_pitch(t, -sp->pitch_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_ROLL_L:
        treegen_turtle_roll(t,  sp->roll_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_ROLL_R:
        treegen_turtle_roll(t, -sp->roll_deg + treegen_rng_jitter(c->rng, jit));
        break;
    case TG_SYM_LEAF:
        leaf_here(c);
        break;
    case TG_SYM_SHRINK:
        shrink(t);
        break;
    default:
        break;   // unknown symbol, no-op. forward-compatible with new alphabet.
    }
}

int treegen_tree_grow_species(treegen_buffer *out, const treegen_species *sp,
                              uint32_t seed) {
    if (!sp || sp->kind == TREEGEN_NONE) return 0;

    int before = out->count;

    treegen_rng rng;
    treegen_rng_seed(&rng, seed);

    // rewrite the grammar.
    static treegen_ruleset rs;     // big-ish; static keeps it off the stack
    static treegen_word word;
    treegen_ruleset_for(&rs, sp->kind);
    treegen_lsystem_rewrite(&rs, sp->iterations, &rng, &word);

    // stand the turtle up. start length/radius come from the species; the
    // grammar's '!' tapers from here.
    tree_ctx c;
    c.sp = sp;
    c.out = out;
    c.rng = &rng;
    c.seg_len_f = (float)sp->seg_len;
    treegen_turtle_init(&c.turtle, c.seg_len_f, (float)sp->trunk_radius + 0.5f);

    // thicken the trunk base separately: the turtle draws a 1-wide centerline,
    // so for fat species we pre-stamp a short solid column at the root.
    if (sp->trunk_radius > 0) {
        int h = sp->min_height < 3 ? 3 : sp->min_height;
        treegen_buffer_column(out, 0, 0, h - 1, 0, sp->trunk_radius, sp->pal.wood);
    }

    for (int i = 0; i < word.len; i++)
        apply_symbol(&c, word.sym[i]);

    // safety net: a degenerate roll sequence could leave a treeless stick. if we
    // barely grew anything, cap the tip with a leaf blob so it's not just a pole.
    if (out->count - before < 4) {
        treegen_buffer_blob(out, (int)(c.turtle.cur.pos.x + 0.5f),
                                 (int)(c.turtle.cur.pos.y + 0.5f),
                                 (int)(c.turtle.cur.pos.z + 0.5f),
                                 sp->leaf_radius, sp->pal.leaf, 100, NULL);
    }

    return out->count - before;
}

int treegen_tree_grow(treegen_buffer *out, treegen_kind kind, uint32_t seed) {
    treegen_species sp = treegen_species_get(kind);
    return treegen_tree_grow_species(out, &sp, seed);
}
