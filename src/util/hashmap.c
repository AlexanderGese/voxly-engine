#include "hashmap.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

static uint64_t mix64(uint64_t x) {
    // splitmix64
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    x ^=  x >> 31;
    return x;
}

void hashmap_init(hashmap *m, size_t initial_cap) {
    if (initial_cap < 16) initial_cap = 16;
    // round up to power of two for fast mask
    size_t c = 1;
    while (c < initial_cap) c <<= 1;
    m->slots = calloc(c, sizeof(hm_slot));
    m->cap   = c;
    m->len   = 0;
}

void hashmap_free(hashmap *m) {
    free(m->slots);
    m->slots = NULL;
    m->cap = 0;
    m->len = 0;
}

static size_t find_slot(const hashmap *m, uint64_t key, int accept_tomb) {
    size_t mask = m->cap - 1;
    size_t i = mix64(key) & mask;
    size_t first_tomb = (size_t)-1;
    while (1) {
        hm_slot *s = &m->slots[i];
        if (s->state == 0) {
            if (accept_tomb && first_tomb != (size_t)-1) return first_tomb;
            return i;
        }
        if (s->state == 2) {
            if (accept_tomb && first_tomb == (size_t)-1) first_tomb = i;
        } else if (s->key == key) {
            return i;
        }
        i = (i + 1) & mask;
    }
}

static void grow(hashmap *m) {
    hm_slot *old = m->slots;
    size_t old_cap = m->cap;
    size_t new_cap = old_cap * 2;
    m->slots = calloc(new_cap, sizeof(hm_slot));
    m->cap = new_cap;
    m->len = 0;
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].state == 1) {
            hashmap_put(m, old[i].key, old[i].val);
        }
    }
    free(old);
}

void hashmap_put(hashmap *m, uint64_t key, void *val) {
    if ((m->len + 1) * 4 > m->cap * 3) grow(m);
    size_t i = find_slot(m, key, 1);
    hm_slot *s = &m->slots[i];
    if (s->state != 1) m->len++;
    s->key = key;
    s->val = val;
    s->state = 1;
}

void *hashmap_get(const hashmap *m, uint64_t key) {
    if (!m->slots) return NULL;
    size_t i = find_slot(m, key, 0);
    hm_slot *s = &m->slots[i];
    return s->state == 1 ? s->val : NULL;
}

int hashmap_has(const hashmap *m, uint64_t key) {
    return hashmap_get(m, key) != NULL;
}

void hashmap_del(hashmap *m, uint64_t key) {
    if (!m->slots) return;
    size_t i = find_slot(m, key, 0);
    hm_slot *s = &m->slots[i];
    if (s->state == 1) {
        s->state = 2;
        s->val = NULL;
        m->len--;
    }
}

size_t hashmap_len(const hashmap *m) { return m->len; }

void hm_iter_init(hm_iter *it, const hashmap *m) {
    it->m = m;
    it->i = 0;
}

int hm_iter_next(hm_iter *it, uint64_t *key, void **val) {
    while (it->i < it->m->cap) {
        const hm_slot *s = &it->m->slots[it->i++];
        if (s->state == 1) {
            if (key) *key = s->key;
            if (val) *val = s->val;
            return 1;
        }
    }
    return 0;
}
