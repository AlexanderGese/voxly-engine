#include "lightprop_test.h"
#include "lightprop_types.h"
#include "lightprop_queue.h"
#include "lightprop_step.h"
#include "lightprop_codec.h"
#include "../../util/log.h"

#include <string.h>
#include <stdlib.h>

// tiny check macro. bumps the failure counter and logs, doesn't abort, so one
// run reports everything that's broken instead of just the first thing.
#define CHECK(cond) do { \
        if (!(cond)) { fails++; LOGE("lightprop selftest FAIL: %s", #cond); } \
    } while (0)

static int test_queue(void) {
    int fails = 0;
    lp_queue *q = malloc(sizeof *q);   // 768KB, too big for the stack
    lp_queue_init(q);

    CHECK(lp_queue_empty(q));
    CHECK(lp_queue_count(q) == 0);

    for (int i = 0; i < 100; i++)
        CHECK(lp_queue_push(q, i, i * 2, i * 3, (uint8_t)(i % 16)));
    CHECK(lp_queue_count(q) == 100);
    CHECK(!lp_queue_empty(q));

    for (int i = 0; i < 100; i++) {
        lp_node n;
        CHECK(lp_queue_pop(q, &n));
        CHECK(n.x == i && n.y == i * 2 && n.z == i * 3);
        CHECK(n.level == (uint8_t)(i % 16));
    }
    CHECK(lp_queue_empty(q));

    // overfill: should report drops, never corrupt.
    lp_queue_reset(q);
    q->dropped = 0;
    for (int i = 0; i < LP_QCAP + 50; i++)
        lp_queue_push(q, i, 0, 0, 1);
    CHECK(q->dropped > 0);
    CHECK(lp_queue_count(q) <= LP_QCAP);

    free(q);
    return fails;
}

static int test_step(void) {
    int fails = 0;

    // plain air step loses exactly 1.
    CHECK(lp_step_falloff(MAX_LIGHT, BLOCK_AIR) == MAX_LIGHT - 1);
    CHECK(lp_step_falloff(1, BLOCK_AIR) == 0);          // can't go below 0
    CHECK(lp_step_falloff(0, BLOCK_AIR) == 0);

    // opaque eats everything.
    CHECK(lp_step_falloff(MAX_LIGHT, BLOCK_STONE) == 0);

    // water attenuates harder (base 1 + extra 2 == 3).
    CHECK(lp_step_falloff(MAX_LIGHT, BLOCK_WATER) == MAX_LIGHT - 3);

    // sky drop: full light going straight down through air keeps full.
    CHECK(lp_step_is_sky_drop(LP_SKY, 3, MAX_LIGHT, BLOCK_AIR) == 1);
    CHECK(lp_step_propagate(LP_SKY, 3, MAX_LIGHT, BLOCK_AIR) == MAX_LIGHT);
    // but sideways still falls off.
    CHECK(lp_step_propagate(LP_SKY, 0, MAX_LIGHT, BLOCK_AIR) == MAX_LIGHT - 1);
    // and non-full sky doesn't get the freebie.
    CHECK(lp_step_is_sky_drop(LP_SKY, 3, MAX_LIGHT - 1, BLOCK_AIR) == 0);
    // block channel never gets the drop.
    CHECK(lp_step_is_sky_drop(LP_BLOCK, 3, MAX_LIGHT, BLOCK_AIR) == 0);

    // opposite-direction pairing the border code relies on (d ^ 1).
    CHECK((0 ^ 1) == 1 && (2 ^ 1) == 3 && (4 ^ 1) == 5);
    for (int d = 0; d < 6; d++) {
        int o = d ^ 1;
        CHECK(LP_DX[d] == -LP_DX[o]);
        CHECK(LP_DY[d] == -LP_DY[o]);
        CHECK(LP_DZ[d] == -LP_DZ[o]);
    }
    return fails;
}

static int test_codec(void) {
    int fails = 0;
    chunk *c = calloc(1, sizeof *c);

    // all zero -> single run, round trips.
    CHECK(lp_codec_roundtrip_ok(c));

    // a realistic-ish pattern: bright sky slab up top, dark below, a torch blob.
    for (int i = 0; i < CHUNK_VOLUME; i++) {
        int y = i / (CHUNK_SIZE_X * CHUNK_SIZE_Z);
        uint8_t sky = y > WORLD_SEA_LEVEL ? MAX_LIGHT : 0;
        uint8_t blk = (i % 997 == 0) ? 14 : 0;
        c->light[i] = (uint8_t)((sky << 4) | (blk & 0x0F));
    }
    CHECK(lp_codec_roundtrip_ok(c));

    // encoded size of the structured data should beat the raw array handily.
    uint8_t *buf = malloc(lp_codec_bound());
    size_t n = lp_codec_encode(c, buf, lp_codec_bound());
    CHECK(n > 0);
    CHECK(n < (size_t)CHUNK_VOLUME);    // RLE actually helped

    // pure noise: worst case, must still round trip even if it doesn't shrink.
    for (int i = 0; i < CHUNK_VOLUME; i++) c->light[i] = (uint8_t)(i * 131 + 7);
    CHECK(lp_codec_roundtrip_ok(c));

    free(buf);
    free(c);
    return fails;
}

int lightprop_selftest(void) {
    int fails = 0;
    fails += test_queue();
    fails += test_step();
    fails += test_codec();
    if (fails == 0) LOGI("lightprop selftest: all good");
    else            LOGE("lightprop selftest: %d failures", fails);
    return fails;
}
