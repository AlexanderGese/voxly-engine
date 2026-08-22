#ifndef SOUND_SOUND_H
#define SOUND_SOUND_H

// sound stub. planned to use openal but i never wired it up. for now
// it just logs what it would play. all events go through this interface
// so switching to real audio later should be painless.

typedef enum {
    SFX_NONE = 0,
    SFX_BLOCK_BREAK,
    SFX_BLOCK_PLACE,
    SFX_FOOTSTEP,
    SFX_JUMP,
    SFX_LAND,
    SFX_HURT,
    SFX_PICKUP,
    SFX_CRAFT,
    SFX_OPEN_CHEST,
    SFX_CLOSE_CHEST,
    SFX_COUNT
} sfx_id;

int  sound_init(void);
void sound_shutdown(void);
void sound_play(sfx_id id);
void sound_play_at(sfx_id id, float x, float y, float z);
void sound_set_volume(float master);
void sound_set_listener(float x, float y, float z, float yaw);

#endif
