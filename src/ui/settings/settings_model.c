#include "settings_model.h"
#include "settings_schema.h"

#include <string.h>

// SETTINGS_ID_COUNT must fit in the dirty bitmask word. if you blow past 32
// settings, widen dirty_mask to uint64_t (or a small array) and fix the shifts.
typedef char settings__mask_fits[(SETTINGS_ID_COUNT <= 32) ? 1 : -1];

static void seed_from_schema(settings_value *bag) {
    const settings_opt *sc = settings_schema();
    for (int i = 0; i < SETTINGS_ID_COUNT; i++)
        bag[i] = sc[i].def;
}

void settings_model_init(settings_model *m) {
    memset(m, 0, sizeof *m);
    seed_from_schema(m->live);
    memcpy(m->work, m->live, sizeof m->work);
    memcpy(m->base, m->live, sizeof m->base);
    m->dirty_mask = 0;
    m->dirty_count = 0;
    m->state = SETTINGS_COMMIT_CLEAN;
}

void settings_model_defaults(settings_model *m) {
    seed_from_schema(m->work);
    settings_model_refresh(m);
}

settings_value *settings_model_work(settings_model *m, settings_id id) {
    if (id < 0 || id >= SETTINGS_ID_COUNT) id = (settings_id)0;
    return &m->work[id];
}

const settings_value *settings_model_live(const settings_model *m, settings_id id) {
    if (id < 0 || id >= SETTINGS_ID_COUNT) id = (settings_id)0;
    return &m->live[id];
}

void settings_model_refresh(settings_model *m) {
    uint32_t mask = 0;
    int count = 0;
    for (int i = 0; i < SETTINGS_ID_COUNT; i++) {
        if (!settings_value_equal(&m->work[i], &m->live[i])) {
            mask |= (uint32_t)1u << i;
            count++;
        }
    }
    m->dirty_mask = mask;
    m->dirty_count = count;

    // refresh never claims APPLIED — that's a one-shot edge set by apply() and
    // cleared by take_applied(). preserve it if it's pending this frame.
    if (m->state != SETTINGS_COMMIT_APPLIED)
        m->state = count ? SETTINGS_COMMIT_DIRTY : SETTINGS_COMMIT_CLEAN;
}

int settings_model_field_dirty(const settings_model *m, settings_id id) {
    if (id < 0 || id >= SETTINGS_ID_COUNT) return 0;
    return (m->dirty_mask >> id) & 1u;
}

void settings_model_snapshot(settings_model *m) {
    memcpy(m->base, m->live, sizeof m->base);
}

void settings_model_apply(settings_model *m) {
    settings_model_refresh(m);
    if (m->dirty_count == 0) {
        // nothing to commit; don't fire the applied edge over a no-op.
        m->state = SETTINGS_COMMIT_CLEAN;
        return;
    }
    memcpy(m->live, m->work, sizeof m->live);
    settings_model_snapshot(m);
    m->dirty_mask = 0;
    m->dirty_count = 0;
    m->state = SETTINGS_COMMIT_APPLIED;
}

void settings_model_revert(settings_model *m) {
    memcpy(m->work, m->base, sizeof m->work);
    settings_model_refresh(m);
}

int settings_model_take_applied(settings_model *m) {
    if (m->state == SETTINGS_COMMIT_APPLIED) {
        m->state = SETTINGS_COMMIT_CLEAN;
        return 1;
    }
    return 0;
}
