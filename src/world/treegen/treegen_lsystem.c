#include "treegen_lsystem.h"
#include <stddef.h>
static void rule_set(treegen_rule *r, treegen_sym pred, float w,
                     const treegen_sym *succ, int n) {
    r->pred = pred;
    r->weight = w;
    r->succ_len = n > TG_RULE_MAX ? TG_RULE_MAX : n;
    for (int i = 0; i < r->succ_len; i++) r->succ[i] = succ[i];
}

static void axiom_set(treegen_ruleset *rs, const treegen_sym *a, int n) {
    rs->axiom_len = n > TG_RULE_MAX ? TG_RULE_MAX : n;
for (int i = 0;
i < rs->axiom_len;
i++) rs->axiom[i] = a[i];
}

void treegen_ruleset_for(treegen_ruleset *rs, treegen_kind kind) {
    rs->rule_count = 0;
    rs->axiom_len = 0;

    switch (kind) {
    case TREEGEN_OAK: {
        // axiom: a trunk segment that recurses. F -> F[branch]F[branch] with
        // two competing productions so limbs aren't perfectly symmetric.
        static const treegen_sym ax[] = { TG_SYM_FWD, TG_SYM_FWD };
        axiom_set(rs, ax, 2);
        static const treegen_sym a[] = {
            TG_SYM_FWD, TG_SYM_PUSH, TG_SYM_PITCH_D, TG_SYM_YAW_L, TG_SYM_SHRINK,
            TG_SYM_FWD, TG_SYM_LEAF, TG_SYM_POP, TG_SYM_FWD, TG_SYM_PUSH,
            TG_SYM_PITCH_D, TG_SYM_YAW_R, TG_SYM_SHRINK, TG_SYM_FWD, TG_SYM_LEAF,
            TG_SYM_POP
        };
        static const treegen_sym b[] = {
            TG_SYM_FWD, TG_SYM_ROLL_L, TG_SYM_PUSH, TG_SYM_PITCH_D, TG_SYM_YAW_L,
            TG_SYM_SHRINK, TG_SYM_FWD, TG_SYM_LEAF, TG_SYM_POP, TG_SYM_FWD
        };
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_FWD, 0.6f, a, (int)(sizeof a / sizeof *a));
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_FWD, 0.4f, b, (int)(sizeof b / sizeof *b));
        break;
    }
    case TREEGEN_PINE: {
        // a strong central leader (one F survives) plus tiered whorls of short
        // down-pitched branches. roll between whorls so they spiral.
        static const treegen_sym ax[] = { TG_SYM_FWD };
        axiom_set(rs, ax, 1);
        static const treegen_sym a[] = {
            TG_SYM_FWD, TG_SYM_PUSH, TG_SYM_PITCH_D, TG_SYM_SHRINK, TG_SYM_FWD,
            TG_SYM_LEAF, TG_SYM_POP, TG_SYM_ROLL_L, TG_SYM_PUSH, TG_SYM_PITCH_D,
            TG_SYM_SHRINK, TG_SYM_FWD, TG_SYM_LEAF, TG_SYM_POP, TG_SYM_ROLL_R,
            TG_SYM_FWD
        };
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_FWD, 1.0f, a, (int)(sizeof a / sizeof *a));
        break;
    }
    case TREEGEN_BIRCH: {
        // tall and lazy. mostly extend straight, occasionally fork late.
        static const treegen_sym ax[] = { TG_SYM_FWD, TG_SYM_FWD, TG_SYM_FWD };
        axiom_set(rs, ax, 3);
        static const treegen_sym a[] = {
            TG_SYM_FWD, TG_SYM_FWD, TG_SYM_PUSH, TG_SYM_YAW_L, TG_SYM_PITCH_D,
            TG_SYM_SHRINK, TG_SYM_FWD, TG_SYM_LEAF, TG_SYM_POP
        };
        static const treegen_sym b[] = { TG_SYM_FWD, TG_SYM_FWD };
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_FWD, 0.35f, a, (int)(sizeof a / sizeof *a));
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_FWD, 0.65f, b, (int)(sizeof b / sizeof *b));
        break;
    }
    case TREEGEN_PALM: {
        // bare trunk (move-without-wood is handled by the interpreter), then at
        // the crown a single burst of arching fronds. only the crown expands.
        static const treegen_sym ax[] = {
            TG_SYM_FWD, TG_SYM_FWD, TG_SYM_FWD, TG_SYM_LEAF
        };
        axiom_set(rs, ax, 4);
        static const treegen_sym a[] = {
            TG_SYM_PUSH, TG_SYM_PITCH_D, TG_SYM_FWD, TG_SYM_PITCH_D, TG_SYM_LEAF,
            TG_SYM_POP, TG_SYM_ROLL_L, TG_SYM_PUSH, TG_SYM_PITCH_D, TG_SYM_FWD,
            TG_SYM_PITCH_D, TG_SYM_LEAF, TG_SYM_POP, TG_SYM_ROLL_L
        };
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_LEAF, 1.0f, a, (int)(sizeof a / sizeof *a));
        break;
    }
    case TREEGEN_BUSH:
    default: {
        // no leader to speak of; just spray short forks from a stub.
        static const treegen_sym ax[] = { TG_SYM_FWD, TG_SYM_LEAF };
        axiom_set(rs, ax, 2);
        static const treegen_sym a[] = {
            TG_SYM_PUSH, TG_SYM_YAW_L, TG_SYM_PITCH_D, TG_SYM_FWD, TG_SYM_LEAF,
            TG_SYM_POP, TG_SYM_PUSH, TG_SYM_YAW_R, TG_SYM_PITCH_D, TG_SYM_FWD,
            TG_SYM_LEAF, TG_SYM_POP
        };
        rule_set(&rs->rules[rs->rule_count++], TG_SYM_LEAF, 1.0f, a, (int)(sizeof a / sizeof *a));
        break;
    }
    }
}

// pick a production for `sym`, weighted, or NULL if the symbol is terminal.
static const treegen_rule *pick_rule(const treegen_ruleset *rs, treegen_sym sym,
                                     treegen_rng *r) {
    float total = 0.0f;
for (int i = 0;
i < rs->rule_count;
i++)
        if (rs->rules[i].pred == sym) total += rs->rules[i].weight;
if (total <= 0.0f) return NULL;
float roll = treegen_rng_f01(r) * total;
for (int i = 0;
i < rs->rule_count;
treegen_word *src = &a, *dst = &b;
a.len = 0;
word_append(&a, rs->axiom, rs->axiom_len);
for (int it = 0;
it < iters;
for (int i = 0;
i < src->len;
i++) out->sym[i] = src->sym[i];
return out->len;
}
