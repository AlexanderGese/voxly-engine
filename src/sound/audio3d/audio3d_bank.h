#ifndef SOUND_AUDIO3D_BANK_H
#define SOUND_AUDIO3D_BANK_H

#include "audio3d_types.h"
#include "../../util/hashmap.h"

// the sound bank. owns all the clips and maps a stable string-ish name (hashed
// to u64) to a clip id. callers register clips once at load and then trigger
// them by name or by the id we hand back.

typedef struct {
    audio3d_clip clips[AUDIO3D_MAX_CLIPS];
    int          count;          // high-water mark of used slots
    hashmap      by_name;        // fnv1a(name) -> (clip_id+1) stuffed in ptr
} audio3d_bank;

void audio3d_bank_init(audio3d_bank *b);
void audio3d_bank_free(audio3d_bank *b);

// register a clip under a name. takes ownership of the clip's samples (moves
// it in). returns the clip id, or -1 if the bank is full. re-registering the
// same name replaces the old clip.
int  audio3d_bank_add(audio3d_bank *b, const char *name, audio3d_clip *clip);

// convenience: build a tone and register it in one go.
int  audio3d_bank_add_tone(audio3d_bank *b, const char *name,
                           float hz, float seconds, float amp);

// look up a clip id by name. returns -1 if unknown.
int  audio3d_bank_find(const audio3d_bank *b, const char *name);

// borrow a clip by id (does not transfer ownership). NULL if id is bogus.
const audio3d_clip *audio3d_bank_clip(const audio3d_bank *b, int id);

// fnv-1a of a name. exposed so callers can pre-hash hot names if they want.
uint64_t audio3d_bank_hash(const char *name);

#endif
