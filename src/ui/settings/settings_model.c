#include "settings_model.h"
#include "settings_schema.h"
#include <string.h>
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
for (int i = 0;
i < SETTINGS_ID_COUNT;
m->dirty_count = count;
if (m->state != SETTINGS_COMMIT_APPLIED)
        m->state = count ? SETTINGS_COMMIT_DIRTY : SETTINGS_COMMIT_CLEAN;
settings_model_refresh(m);
