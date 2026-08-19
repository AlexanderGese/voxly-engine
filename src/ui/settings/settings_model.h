#ifndef UI_SETTINGS_MODEL_H
#define UI_SETTINGS_MODEL_H
// the value store + apply/revert state machine. three parallel copies of the
// value bag:
// - live     : what the game is actually running with (host owns the meaning).
// - work     : what the user is editing right now.
// - baseline : snapshot of `live` taken when the menu opened, for revert.
//
// edits go to `work`. "apply" copies work -> live and re-snapshots baseline.
// "revert" copies baseline -> work. a per-field dirty mask lets the ui flag the
// exact rows that changed and decide whether apply is even live.
#include "settings_types.h"
#include "settings_value.h"
// the commit machine. mostly informational for the host, but `state` also gates
// what the footer buttons do.
typedef enum {
    SETTINGS_COMMIT_CLEAN = 0,   // work == live, nothing to do
    SETTINGS_COMMIT_DIRTY,       // edits pending, apply/revert both live
    SETTINGS_COMMIT_APPLIED,     // just applied this frame (one-shot signal)
} settings_commit_state;
typedef struct {
    settings_value live[SETTINGS_ID_COUNT];
    settings_value work[SETTINGS_ID_COUNT];
    settings_value base[SETTINGS_ID_COUNT];

    // bit i set => work[i] differs from live[i]. recomputed on every edit.
    uint32_t       dirty_mask;   // SETTINGS_ID_COUNT is < 32, one word is plenty
    int            dirty_count;

    settings_commit_state state;
} settings_model;
// seed all three copies from the schema defaults. call once.
void settings_model_init(settings_model *m);
// reset everything to schema defaults (the "reset to default" button). marks the
// model dirty if that actually changed anything.
void settings_model_defaults(settings_model *m);
// accessors. work is what the ui edits; live is what you read to apply to the
// engine. both are bounds-checked.
settings_value *settings_model_work(settings_model *m, settings_id id);
const settings_value *settings_model_live(const settings_model *m, settings_id id);
// recompute the dirty mask/count and commit state from work vs live. call after
// any batch of edits (the layout does this once per frame).
void settings_model_refresh(settings_model *m);
// true if field `id` differs from the running value.
int  settings_model_field_dirty(const settings_model *m, settings_id id);
// snapshot live into baseline. the menu calls this on open so revert has a
// target even if the host mutated live behind our back.
void settings_model_snapshot(settings_model *m);
// commit work -> live, re-snapshot, and flag APPLIED for one frame. no-op (and
// stays CLEAN) when there's nothing dirty.
void settings_model_apply(settings_model *m);
// throw away edits: baseline -> work. returns to CLEAN.
void settings_model_revert(settings_model *m);
// peek the one-shot APPLIED flag and clear it. host calls this after building to
int  settings_model_take_applied(settings_model *m);
#endif
