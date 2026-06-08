#ifndef UTIL_HASHMAP_H
#define UTIL_HASHMAP_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
    uint64_t key;
    void    *val;
    uint8_t  state;     // 0 empty, 1 used, 2 tomb
} hm_slot;
typedef struct {
    hm_slot *slots;
    size_t   cap;
    size_t   len;
} hashmap;
#endif
