#include "fishing_bite.h"

// raw wait bounds before quality/lure adjustments, in seconds. these bracket
// the classic 5..30s feel; we squeeze toward the low end on good water.
#define BITE_WAIT_MIN   5.0f
#define BITE_WAIT_MAX  30.0f

// hard floor so even a maxed rod on perfect water still makes you wait a touch.
#define BITE_WAIT_FLOOR 1.5f

void fishing_bite_begin(fishing_bite *bt, fishing_rng *r,
                        const fishing_rod *rod, float quality) {
    // base roll across the full band.
    float wait = fishing_rng_frange(r, BITE_WAIT_MIN, BITE_WAIT_MAX);

    // good water pulls the wait down: at quality 1 we keep ~55% of it, at the
    // 0.05 floor we keep nearly all of it.
    float qscale = 1.0f - 0.45f * quality;
    wait *= qscale;

    // lure shaves a flat amount on top.
    wait -= fishing_rod_lure_bonus(rod);
    if (wait < BITE_WAIT_FLOOR) wait = BITE_WAIT_FLOOR;

    bt->wait        = wait;
    // the approach is short and a little random so you can't perfectly time it.
    bt->lure_time   = fishing_rng_frange(r, 0.6f, 1.1f);
    // the actual grab window. tightens slightly on better water (fish are
    // jumpier where they're plentiful). never below a fair 0.4s though.
    bt->bite_window = fishing_rng_frange(r, 0.5f, 0.9f) - quality * 0.1f;
    if (bt->bite_window < 0.4f) bt->bite_window = 0.4f;
    bt->elapsed = 0.0f;
    bt->misses  = 0;
}

int fishing_bite_tick_wait(fishing_bite *bt, float dt) {
    bt->wait -= dt;
    if (bt->wait <= 0.0f) {
        bt->wait = 0.0f;
        bt->elapsed = 0.0f;
        return 1;
    }
    return 0;
}

int fishing_bite_tick_lure(fishing_bite *bt, float dt) {
    bt->elapsed += dt;
    if (bt->elapsed >= bt->lure_time) {
        bt->elapsed = 0.0f;
        return 1;
    }
    return 0;
}

int fishing_bite_tick_window(fishing_bite *bt, float dt) {
    bt->elapsed += dt;
    if (bt->elapsed >= bt->bite_window) {
        bt->misses++;
        bt->elapsed = 0.0f;
        return 1;   // window closed unclaimed
    }
    return 0;
}

float fishing_bite_lure_progress(const fishing_bite *bt) {
    if (bt->lure_time <= 0.0f) return 1.0f;
    float p = bt->elapsed / bt->lure_time;
    return p < 0.0f ? 0.0f : (p > 1.0f ? 1.0f : p);
}
