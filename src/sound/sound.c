#include "sound.h"
#include "../util/log.h"

static float g_vol = 1.0f;
static float g_lx, g_ly, g_lz, g_lyaw;

static const char *name_of(sfx_id id) {
    switch (id) {
    case SFX_BLOCK_BREAK: return "break";
    case SFX_BLOCK_PLACE: return "place";
    case SFX_FOOTSTEP:    return "step";
    case SFX_JUMP:        return "jump";
    case SFX_LAND:        return "land";
    case SFX_HURT:        return "hurt";
    case SFX_PICKUP:      return "pickup";
    case SFX_CRAFT:       return "craft";
    case SFX_OPEN_CHEST:  return "chest_open";
    case SFX_CLOSE_CHEST: return "chest_close";
    default:              return "?";
    }
}

int sound_init(void) {
    LOGI("sound: openal was planned but not wired up. running silent.");
    return 1;
}

void sound_shutdown(void) {}

void sound_play(sfx_id id) {
    LOGD("sfx: %s (vol %.2f)", name_of(id), g_vol);
}

void sound_play_at(sfx_id id, float x, float y, float z) {
    (void)x; (void)y; (void)z;
    sound_play(id);
}

void sound_set_volume(float master) { g_vol = master; }

void sound_set_listener(float x, float y, float z, float yaw) {
    g_lx = x; g_ly = y; g_lz = z; g_lyaw = yaw;
}
