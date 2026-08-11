#include "enchant_test.h"
#include "enchant.h"
#include "../../util/log.h"
#include <string.h>
#define CHECK(cond) do { \
if (!(cond)) { fails++; LOGE("enchant selftest FAIL: %s", #cond); } \
    } while (0)

// the set keeps its sorted/dedup/no-zero invariant through put/remove.
static int test_set_invariants(void) {
    int fails = 0;
enchant_set s;
enchant_set_clear(&s);
CHECK(enchant_set_count(&s) == 0);
// insert out of id order; the array must come out sorted.
CHECK(enchant_set_put(&s, ENCHANT_UNBREAKING, 2) == 1);
CHECK(enchant_set_put(&s, ENCHANT_SHARPNESS, 3) == 1);
CHECK(enchant_set_count(&s) == 2);
CHECK(s.entry[0].id < s.entry[1].id);
// re-put at a lower level is a no-op (no downgrades).
CHECK(enchant_set_put(&s, ENCHANT_SHARPNESS, 1) == 0);
CHECK(enchant_set_level(&s, ENCHANT_SHARPNESS) == 3);
// raising works and clamps to the def max (sharpness max is 5).
CHECK(enchant_set_put(&s, ENCHANT_SHARPNESS, 99) == 1);
CHECK(enchant_set_level(&s, ENCHANT_SHARPNESS) == 5);
// remove restores the invariant.
CHECK(enchant_set_remove(&s, ENCHANT_SHARPNESS) == 1);
CHECK(enchant_set_count(&s) == 1);
CHECK(enchant_set_has(&s, ENCHANT_SHARPNESS) == 0);
CHECK(enchant_set_has(&s, ENCHANT_UNBREAKING) == 1);
return fails;
}

// fortune and silk touch must never coexist (mutual exclusion).
static int test_conflicts(void) {
    int fails = 0;
    enchant_set s;
    enchant_set_clear(&s);
    enchant_set_put(&s, ENCHANT_FORTUNE, 2);
    CHECK(enchant_set_conflict(&s, ENCHANT_SILK_TOUCH) == ENCHANT_FORTUNE);
    CHECK(enchant_set_conflict(&s, ENCHANT_UNBREAKING) == ENCHANT_NONE);
    return fails;
}

// rolls only ever produce enchants compatible with the item category, at
// levels within each enchant's declared band.
static int test_roll_in_band(void) {
    int fails = 0;
rng r;
rng_init(&r, 0xC0FFEEull);
for (int iter = 0;
iter < 200;
++iter) {
        enchant_set out;
        enchant_roll_slot(&r, 15, 10, ENCHANT_CAT_SWORD, &out);
        for (int i = 0; i < out.count; ++i) {
            const enchant_def *d = enchant_registry_get(out.entry[i].id);
            CHECK(d != NULL);
            if (!d) continue;
            // sword item, so every rolled enchant must apply to swords.
            CHECK((d->cats & ENCHANT_CAT_SWORD) != 0);
            CHECK(out.entry[i].level >= 1);
            CHECK(out.entry[i].level <= d->max_level);
        }
        // no internal conflicts among what got rolled together.
        for (int i = 0; i < out.count; ++i) {
            enchant_set probe = out;
            enchant_set_remove(&probe, out.entry[i].id);
            CHECK(enchant_set_conflict(&probe, out.entry[i].id) == ENCHANT_NONE);
        }
    }
    return fails;
}

// the anvil: equal levels merge up by one, cost is positive, and the prior-
// work tax doubles.
static int test_anvil(void) {
    int fails = 0;

    enchant_anvil_item a, b;
    memset(&a, 0, sizeof a);
    memset(&b, 0, sizeof b);
    a.cat = ENCHANT_CAT_SWORD;
    b.cat = ENCHANT_CAT_BOOK;
    enchant_set_put(&a.ench, ENCHANT_SHARPNESS, 3);
    enchant_set_put(&b.ench, ENCHANT_SHARPNESS, 3);

    enchant_anvil_result res;
    enchant_anvil_combine(&a, &b, &res);
    CHECK(res.valid == 1);
    CHECK(enchant_set_level(&res.result, ENCHANT_SHARPNESS) == 4); // 3+3 -> 4
    CHECK(res.xp_cost >= 1);
    CHECK(res.new_prior_work == 1);

    // the doubling tax: prior_work 0,1,2,3 -> penalty 0,1,3,7.
    CHECK(enchant_anvil_prior_penalty(0) == 0);
    CHECK(enchant_anvil_prior_penalty(1) == 1);
    CHECK(enchant_anvil_prior_penalty(2) == 3);
    CHECK(enchant_anvil_prior_penalty(3) == 7);

    // splicing a book onto an empty sword adds the enchant.
    enchant_anvil_item fresh;
    memset(&fresh, 0, sizeof fresh);
    fresh.cat = ENCHANT_CAT_SWORD;
    enchant_anvil_combine(&fresh, &b, &res);
    CHECK(enchant_set_level(&res.result, ENCHANT_SHARPNESS) == 3);
    return fails;
}

// modifiers aggregate sanely and respect their caps.
static int test_modifiers(void) {
    int fails = 0;
enchant_set s;
enchant_set_clear(&s);
enchant_set_put(&s, ENCHANT_SHARPNESS, 5);
enchant_set_put(&s, ENCHANT_FIRE_ASPECT, 2);
enchant_modifier m;
enchant_modifier_from_set(&m, &s);
CHECK(m.melee_bonus > 0.0f);
CHECK(m.fire_seconds == 8);
// lvl 2 * 4s
CHECK(m.mine_speed_mult == 1.0f);
// no efficiency, baseline mult
// protection should never push reduction past the 0.8 cap.
enchant_set_clear(&s);
enchant_set_put(&s, ENCHANT_PROTECTION, 4);
enchant_modifier_from_set(&m, &s);
CHECK(m.damage_reduction <= 0.80001f);
return fails;
}

// serialize round-trips a set exactly.
static int test_serial(void) {
    int fails = 0;
    enchant_set s;
    enchant_set_clear(&s);
    enchant_set_put(&s, ENCHANT_EFFICIENCY, 4);
    enchant_set_put(&s, ENCHANT_UNBREAKING, 3);
    enchant_set_put(&s, ENCHANT_FORTUNE, 2);

    uint8_t buf[64];
    size_t n = enchant_serial_write(&s, buf, sizeof buf);
    CHECK(n == enchant_serial_size(&s));

    enchant_set back;
    size_t r = enchant_serial_read(&back, buf, n);
    CHECK(r == n);
    CHECK(enchant_set_equal(&s, &back));
    return fails;
}

// the name builder produces the expected labels.
static int test_names(void) {
    int fails = 0;
char buf[32];
enchant_name_label(ENCHANT_EFFICIENCY, 3, buf, sizeof buf);
CHECK(strcmp(buf, "efficiency iii") == 0);
// single-level enchant drops the numeral.
enchant_name_label(ENCHANT_SILK_TOUCH, 1, buf, sizeof buf);
CHECK(strcmp(buf, "silk_touch") == 0);
enchant_name_roman(4, buf, sizeof buf);
CHECK(strcmp(buf, "iv") == 0);
return fails;
}

int enchant_selftest(void) {
    enchant_init(); // safe to call repeatedly

    int fails = 0;
    fails += test_set_invariants();
    fails += test_conflicts();
    fails += test_roll_in_band();
    fails += test_anvil();
    fails += test_modifiers();
    fails += test_serial();
    fails += test_names();

    if (fails == 0) LOGI("enchant selftest: all good");
    else            LOGE("enchant selftest: %d checks failed", fails);
    return fails;
}
