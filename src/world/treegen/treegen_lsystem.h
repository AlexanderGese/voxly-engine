#ifndef WORLD_TREEGEN_LSYSTEM_H
#define WORLD_TREEGEN_LSYSTEM_H

#include "treegen_types.h"
#include "treegen_rand.h"

// the l-system itself: an axiom plus a handful of stochastic production rules,
// rewritten n times into a flat symbol string the turtle later walks. we keep
// the alphabet small (treegen_sym) and store the rewritten word as a byte array
// so we don't pay for a parser. productions are picked per occurrence with the
// plant rng, which is what gives two oaks of the same seed different limbs.

// a rule replaces one symbol with a sequence. a symbol can carry several rules;
// we roll weight to choose. successor lengths are bounded by TG_RULE_MAX.
#define TG_RULE_MAX   24

typedef struct {
    treegen_sym pred;                 // symbol this rule matches
    float       weight;               // relative odds among rules for pred
    treegen_sym succ[TG_RULE_MAX];    // replacement symbols
    int         succ_len;
} treegen_rule;

#define TG_RULESET_MAX  16

typedef struct {
    treegen_sym axiom[TG_RULE_MAX];
    int         axiom_len;
    treegen_rule rules[TG_RULESET_MAX];
    int         rule_count;
} treegen_ruleset;

// the rewritten word. fixed cap; rewrite stops expanding once it would overflow
// so a runaway grammar degrades gracefully into a stubbier tree instead of
// trashing memory.
#define TG_WORD_MAX  4096

typedef struct {
    treegen_sym sym[TG_WORD_MAX];
    int len;
} treegen_word;

// build the grammar for a species. picks axiom + rules from a small table.
void treegen_ruleset_for(treegen_ruleset *rs, treegen_kind kind);

// rewrite the axiom `iters` times under rs, choosing stochastic rules with r.
// result lands in `out`. returns out->len (clamped to TG_WORD_MAX).
int  treegen_lsystem_rewrite(const treegen_ruleset *rs, int iters,
                             treegen_rng *r, treegen_word *out);

#endif
