#include "audio3d_bank.h"
#include "audio3d_clip.h"
#include "../../util/log.h"

#include <string.h>

uint64_t audio3d_bank_hash(const char *name) {
    // fnv-1a 64. same constants everyone uses. empty/null hashes to the basis.
    uint64_t h = 1469598103934665603ULL;
    if (!name) return h;
    for (const unsigned char *p = (const unsigned char *)name; *p; p++) {
        h ^= (uint64_t)*p;
        h *= 1099511628211ULL;
    }
    return h;
}

void audio3d_bank_init(audio3d_bank *b) {
    if (!b) return;
    memset(b->clips, 0, sizeof(b->clips));
    b->count = 0;
    hashmap_init(&b->by_name, 64);
}

void audio3d_bank_free(audio3d_bank *b) {
    if (!b) return;
    for (int i = 0; i < b->count; i++)
        audio3d_clip_free(&b->clips[i]);
    b->count = 0;
    hashmap_free(&b->by_name);
}

// we store id+1 in the void* so that a real 0 id doesnt look like "absent".
static int bank_slot_for(audio3d_bank *b, const char *name) {
    uint64_t key = audio3d_bank_hash(name);
    void *v = hashmap_get(&b->by_name, key);
    if (v) return (int)((intptr_t)v) - 1;     // reuse existing slot
    return -1;
}

int audio3d_bank_add(audio3d_bank *b, const char *name, audio3d_clip *clip) {
    if (!b || !clip || !clip->in_use) return -1;

    int id = bank_slot_for(b, name);
    if (id >= 0) {
        // name already known, replace. drop the old samples first.
        audio3d_clip_free(&b->clips[id]);
        b->clips[id] = *clip;
        memset(clip, 0, sizeof(*clip));       // we own them now
        return id;
    }

    if (b->count >= AUDIO3D_MAX_CLIPS) {
        LOGW("audio3d bank full, dropping clip '%s'", name ? name : "?");
        return -1;
    }
    id = b->count++;
    b->clips[id] = *clip;
    memset(clip, 0, sizeof(*clip));

    uint64_t key = audio3d_bank_hash(name);
    hashmap_put(&b->by_name, key, (void *)(intptr_t)(id + 1));
    return id;
}

int audio3d_bank_add_tone(audio3d_bank *b, const char *name,
                          float hz, float seconds, float amp) {
    audio3d_clip c;
    memset(&c, 0, sizeof(c));
    if (audio3d_clip_make_tone(&c, hz, seconds, AUDIO3D_SAMPLE_RATE, amp) != 0)
        return -1;
    int id = audio3d_bank_add(b, name, &c);
    if (id < 0) audio3d_clip_free(&c);        // add failed, dont leak
    return id;
}

int audio3d_bank_find(const audio3d_bank *b, const char *name) {
    if (!b) return -1;
    uint64_t key = audio3d_bank_hash(name);
    void *v = hashmap_get(&b->by_name, key);
    if (!v) return -1;
    return (int)((intptr_t)v) - 1;
}

const audio3d_clip *audio3d_bank_clip(const audio3d_bank *b, int id) {
    if (!b || id < 0 || id >= b->count) return NULL;
    if (!b->clips[id].in_use) return NULL;
    return &b->clips[id];
}
