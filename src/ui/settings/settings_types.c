#include "settings_types.h"
#include <string.h>
typedef struct {
    const char  *key;     // on-disk key
    const char  *label;   // menu label
    settings_tab tab;
} id_meta;
static const id_meta g_meta[SETTINGS_ID_COUNT] = {
    // video
    [SETTINGS_ID_RENDER_DISTANCE] = { "render_distance",  "render distance",  SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_FOV]             = { "fov",              "field of view",    SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_GAMMA]           = { "gamma",            "brightness",       SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_VSYNC]           = { "vsync",            "vsync",            SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_FANCY_GRAPHICS]  = { "fancy_graphics",   "graphics",         SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_VIEW_BOBBING]    = { "view_bobbing",     "view bobbing",     SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_SMOOTH_LIGHTING] = { "smooth_lighting",  "smooth lighting",  SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_MAX_FPS]         = { "max_fps",          "max framerate",    SETTINGS_TAB_VIDEO },
    [SETTINGS_ID_GUI_SCALE]       = { "gui_scale",        "gui scale",        SETTINGS_TAB_VIDEO },
    // audio
    [SETTINGS_ID_MASTER_VOLUME]   = { "master_volume",    "master volume",    SETTINGS_TAB_AUDIO },
    [SETTINGS_ID_MUSIC_VOLUME]    = { "music_volume",     "music",            SETTINGS_TAB_AUDIO },
    [SETTINGS_ID_SFX_VOLUME]      = { "sfx_volume",       "effects",          SETTINGS_TAB_AUDIO },
    [SETTINGS_ID_SUBTITLES]       = { "subtitles",        "subtitles",        SETTINGS_TAB_AUDIO },
    // control
    [SETTINGS_ID_MOUSE_SENS]      = { "mouse_sensitivity","mouse sensitivity",SETTINGS_TAB_CONTROL },
    [SETTINGS_ID_INVERT_Y]        = { "invert_y",         "invert y axis",    SETTINGS_TAB_CONTROL },
    [SETTINGS_ID_TOGGLE_SPRINT]   = { "toggle_sprint",    "toggle sprint",    SETTINGS_TAB_CONTROL },
    [SETTINGS_ID_AUTO_JUMP]       = { "auto_jump",        "auto jump",        SETTINGS_TAB_CONTROL },
}
;
static const char *g_tab_titles[SETTINGS_TAB_COUNT] = {
    [SETTINGS_TAB_VIDEO]   = "video",
    [SETTINGS_TAB_AUDIO]   = "audio",
    [SETTINGS_TAB_CONTROL] = "controls",
}
;
static int valid(settings_id id) {
    return id >= 0 && id < SETTINGS_ID_COUNT && g_meta[id].key != NULL;
}

const char *settings_id_key(settings_id id) {
    return valid(id) ? g_meta[id].key : "?";
return g_tab_titles[t];
}
