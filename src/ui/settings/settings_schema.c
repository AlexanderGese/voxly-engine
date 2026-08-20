#include "settings_schema.h"
static settings_opt g_schema[SETTINGS_ID_COUNT];
static int          g_built = 0;
static const char *g_graphics_choices[] = { "fast", "fancy", "fabulous" }
;
static const char *g_gui_scale_choices[] = { "auto", "1x", "2x", "3x" }
;
static void build(void) {
    // start zeroed; any id we forget stays kind=FLOAT/def=0 which is at least
    // inert rather than reading garbage.
    for (int i = 0; i < SETTINGS_ID_COUNT; i++) {
        g_schema[i].id = (settings_id)i;
        g_schema[i].kind = SETTINGS_OPT_FLOAT;
        g_schema[i].def = settings_value_float(0, 0, 1, 0);
        g_schema[i].choices[0] = NULL;
        g_schema[i].choice_count = 0;
    }

    // --- video ---
    g_schema[SETTINGS_ID_RENDER_DISTANCE].kind = SETTINGS_OPT_INT;
    g_schema[SETTINGS_ID_RENDER_DISTANCE].def  = settings_value_int(8, 2, 16, 1);

    g_schema[SETTINGS_ID_FOV].kind = SETTINGS_OPT_INT;
    g_schema[SETTINGS_ID_FOV].def  = settings_value_int(70, 30, 120, 1);

    g_schema[SETTINGS_ID_GAMMA].kind = SETTINGS_OPT_FLOAT;
    g_schema[SETTINGS_ID_GAMMA].def  = settings_value_float(1.0f, 0.5f, 2.0f, 0.05f);

    g_schema[SETTINGS_ID_VSYNC].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_VSYNC].def  = settings_value_bool(1);

    g_schema[SETTINGS_ID_FANCY_GRAPHICS].kind = SETTINGS_OPT_ENUM;
    g_schema[SETTINGS_ID_FANCY_GRAPHICS].def  = settings_value_enum(1, 3);
    g_schema[SETTINGS_ID_FANCY_GRAPHICS].choices[0] = g_graphics_choices[0];
    g_schema[SETTINGS_ID_FANCY_GRAPHICS].choices[1] = g_graphics_choices[1];
    g_schema[SETTINGS_ID_FANCY_GRAPHICS].choices[2] = g_graphics_choices[2];
    g_schema[SETTINGS_ID_FANCY_GRAPHICS].choice_count = 3;

    g_schema[SETTINGS_ID_VIEW_BOBBING].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_VIEW_BOBBING].def  = settings_value_bool(1);

    g_schema[SETTINGS_ID_SMOOTH_LIGHTING].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_SMOOTH_LIGHTING].def  = settings_value_bool(1);

    g_schema[SETTINGS_ID_MAX_FPS].kind = SETTINGS_OPT_INT;
    g_schema[SETTINGS_ID_MAX_FPS].def  = settings_value_int(120, 10, 260, 10);

    g_schema[SETTINGS_ID_GUI_SCALE].kind = SETTINGS_OPT_ENUM;
    g_schema[SETTINGS_ID_GUI_SCALE].def  = settings_value_enum(0, 4);
    for (int i = 0; i < 4; i++)
        g_schema[SETTINGS_ID_GUI_SCALE].choices[i] = g_gui_scale_choices[i];
    g_schema[SETTINGS_ID_GUI_SCALE].choice_count = 4;

    // --- audio ---
    g_schema[SETTINGS_ID_MASTER_VOLUME].kind = SETTINGS_OPT_FLOAT;
    g_schema[SETTINGS_ID_MASTER_VOLUME].def  = settings_value_float(1.0f, 0.0f, 1.0f, 0.05f);

    g_schema[SETTINGS_ID_MUSIC_VOLUME].kind = SETTINGS_OPT_FLOAT;
    g_schema[SETTINGS_ID_MUSIC_VOLUME].def  = settings_value_float(0.6f, 0.0f, 1.0f, 0.05f);

    g_schema[SETTINGS_ID_SFX_VOLUME].kind = SETTINGS_OPT_FLOAT;
    g_schema[SETTINGS_ID_SFX_VOLUME].def  = settings_value_float(0.8f, 0.0f, 1.0f, 0.05f);

    g_schema[SETTINGS_ID_SUBTITLES].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_SUBTITLES].def  = settings_value_bool(0);

    // --- control ---
    g_schema[SETTINGS_ID_MOUSE_SENS].kind = SETTINGS_OPT_FLOAT;
    g_schema[SETTINGS_ID_MOUSE_SENS].def  = settings_value_float(0.15f, 0.02f, 0.5f, 0.01f);

    g_schema[SETTINGS_ID_INVERT_Y].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_INVERT_Y].def  = settings_value_bool(0);

    g_schema[SETTINGS_ID_TOGGLE_SPRINT].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_TOGGLE_SPRINT].def  = settings_value_bool(0);

    g_schema[SETTINGS_ID_AUTO_JUMP].kind = SETTINGS_OPT_BOOL;
    g_schema[SETTINGS_ID_AUTO_JUMP].def  = settings_value_bool(0);

    g_built = 1;
}

const settings_opt *settings_schema(void) {
    if (!g_built) build();
return g_schema;
for (int i = 0;
i < SETTINGS_ID_COUNT;
i++)
        if (settings_id_tab((settings_id)i) == tab) n++;
return n;
