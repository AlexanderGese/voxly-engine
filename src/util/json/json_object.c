#include "json_object.h"
#include "json_value.h"

#include <stdlib.h>
#include <string.h>

#include "../darray.h"

json_value json_object(void) {
    json_value v = json_null();
    v.kind = JSON_OBJECT;
    v.as.obj = NULL;
    return v;
}

size_t json_object_len(const json_value *v) {
    if (!v || v->kind != JSON_OBJECT) return 0;
    return darr_len(v->as.obj);
}

// internal: index of a key or -1. shared by get/has/set/remove so the scan
// logic lives in exactly one spot.
static long find_key(const json_value *v, const char *key) {
    if (!v || v->kind != JSON_OBJECT || !key) return -1;
    size_t n = darr_len(v->as.obj);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(v->as.obj[i].key, key) == 0) return (long)i;
    }
    return -1;
}

void json_object_set(json_value *v, const char *key, json_value val) {
    if (!v || v->kind != JSON_OBJECT || !key) {
        json_free(&val);
        return;
    }
    long at = find_key(v, key);
    if (at >= 0) {
        // replace in place. free the old value first, keep the key string.
        json_free(&v->as.obj[at].val);
        v->as.obj[at].val = val;
        return;
    }
    char *kdup = (char *)malloc(strlen(key) + 1);
    if (!kdup) { json_free(&val); return; }
    strcpy(kdup, key);
    json_member m;
    m.key = kdup;
    m.val = val;
    darr_push(v->as.obj, m);
}

json_value *json_object_get(json_value *v, const char *key) {
    long at = find_key(v, key);
    return at < 0 ? NULL : &v->as.obj[at].val;
}

const json_value *json_object_get_const(const json_value *v, const char *key) {
    long at = find_key(v, key);
    return at < 0 ? NULL : &v->as.obj[at].val;
}

int json_object_has(const json_value *v, const char *key) {
    return find_key(v, key) >= 0;
}

void json_object_remove(json_value *v, const char *key) {
    long at = find_key(v, key);
    if (at < 0) return;
    size_t i = (size_t)at;
    size_t n = darr_len(v->as.obj);
    free(v->as.obj[i].key);
    json_free(&v->as.obj[i].val);
    if (i + 1 < n) {
        memmove(&v->as.obj[i], &v->as.obj[i + 1],
                (n - i - 1) * sizeof(json_member));
    }
    darr_hdr(v->as.obj)->len = n - 1;
}

int json_object_at(const json_value *v, size_t i,
                   const char **out_key, const json_value **out_val) {
    if (!v || v->kind != JSON_OBJECT) return 0;
    if (i >= darr_len(v->as.obj)) return 0;
    if (out_key) *out_key = v->as.obj[i].key;
    if (out_val) *out_val = &v->as.obj[i].val;
    return 1;
}
