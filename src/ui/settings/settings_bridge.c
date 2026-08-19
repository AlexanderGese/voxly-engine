#include "settings_bridge.h"
#include <string.h>
// helper: overwrite an int/enum/bool field's payload in `m->live` while keeping
// its schema-provided range/kind intact, then clamp. we never touch kind/lo/hi
// because those come from the schema, not the engine.
static void set_int(settings_model *m, settings_id id, int v) {
    // the model keeps live[] public precisely so bridges like this can seed it
    // without going through the edit path. range/kind stay schema-owned; we only
    // poke the payload and re-clamp.
    if (id < 0 || id >= SETTINGS_ID_COUNT) return;
    settings_value *lv = &m->live[id];
    lv->i = v;
    lv->f = (float)v;
    settings_value_clamp(lv);
}

static void set_float(settings_model *m, settings_id id, float v) {
    if (id < 0 || id >= SETTINGS_ID_COUNT) return;
settings_value *lv = &m->live[id];
lv->f = v;
settings_value_clamp(lv);
}

static int get_int(const settings_model *m, settings_id id) {
    return settings_model_live(m, id)->i;
}

static float get_float(const settings_model *m, settings_id id) {
    return settings_model_live(m, id)->f;
}

void settings_bridge_pull(settings_model *m, const game_settings *gs) {
    if (!gs) return;

    set_int  (m, SETTINGS_ID_RENDER_DISTANCE, gs->render_distance);
    set_int  (m, SETTINGS_ID_FOV,             (int)(gs->fov + 0.5f));
    set_int  (m, SETTINGS_ID_VSYNC,           gs->vsync ? 1 : 0);
    set_int  (m, SETTINGS_ID_VIEW_BOBBING,    gs->view_bobbing ? 1 : 0);
    set_int  (m, SETTINGS_ID_SMOOTH_LIGHTING, gs->smooth_lighting ? 1 : 0);
    set_int  (m, SETTINGS_ID_MAX_FPS,         gs->max_fps);

    // fancy_graphics is a bool in the engine but a 3-way enum here. map the bool
    // onto fast(0)/fancy(1); we leave "fabulous" reachable only from our menu.
    set_int  (m, SETTINGS_ID_FANCY_GRAPHICS,  gs->fancy_graphics ? 1 : 0);

    // gui_scale: engine stores an integer (0=auto, 1..); our enum indices line up
    // closely enough (0 auto, 1 -> 1x, ...). clamp handles overshoot.
    set_int  (m, SETTINGS_ID_GUI_SCALE,       gs->gui_scale);

    set_float(m, SETTINGS_ID_MASTER_VOLUME,   gs->master_volume);
    set_float(m, SETTINGS_ID_MUSIC_VOLUME,    gs->music_volume);
    set_float(m, SETTINGS_ID_SFX_VOLUME,      gs->sfx_volume);

    set_float(m, SETTINGS_ID_MOUSE_SENS,      gs->mouse_sensitivity);

    // re-mirror live into work + baseline so the freshly-pulled values are the
    // clean state the menu diffs against.
    memcpy(m->work, m->live, sizeof m->work);
    memcpy(m->base, m->live, sizeof m->base);
    settings_model_refresh(m);
}

void settings_bridge_push(const settings_model *m, game_settings *gs) {
    if (!gs) return;
gs->render_distance  = get_int  (m, SETTINGS_ID_RENDER_DISTANCE);
gs->fov              = get_float(m, SETTINGS_ID_FOV);
gs->vsync            = get_int  (m, SETTINGS_ID_VSYNC);
gs->view_bobbing     = get_int  (m, SETTINGS_ID_VIEW_BOBBING);
gs->smooth_lighting  = get_int  (m, SETTINGS_ID_SMOOTH_LIGHTING);
gs->max_fps          = get_int  (m, SETTINGS_ID_MAX_FPS);
// collapse the 3-way quality enum back to the engine's bool: fast => off,
// anything fancier => on. lossy but the engine only has the one bit.
gs->fancy_graphics   = get_int  (m, SETTINGS_ID_FANCY_GRAPHICS) > 0 ? 1 : 0;
gs->gui_scale        = get_int  (m, SETTINGS_ID_GUI_SCALE);
gs->master_volume    = get_float(m, SETTINGS_ID_MASTER_VOLUME);
gs->music_volume     = get_float(m, SETTINGS_ID_MUSIC_VOLUME);
gs->sfx_volume       = get_float(m, SETTINGS_ID_SFX_VOLUME);
gs->mouse_sensitivity = get_float(m, SETTINGS_ID_MOUSE_SENS);
}
