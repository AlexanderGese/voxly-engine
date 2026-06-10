#include "json_query.h"
#include "json_value.h"
#include "json_array.h"
#include "json_object.h"

#include <stdlib.h>
#include <string.h>

// walk one segment of the path forward and step `cur` into the child. returns
// the position just past the consumed segment, or NULL if the step fails. on a
// resolution miss we set *cur to NULL and still return the advanced pointer so
// the caller can tell "bad value" from "bad syntax" if it ever cares (it
// doesnt today, but the shape is there).
static const char *step(const json_value **cur, const char *p) {
    if (*p == '[') {
        // numeric index. parse digits until the matching ']'.
        p++;
        if (*p < '0' || *p > '9') { *cur = NULL; return p; }
        size_t idx = 0;
        while (*p >= '0' && *p <= '9') {
            idx = idx * 10 + (size_t)(*p - '0');
            p++;
        }
        if (*p != ']') { *cur = NULL; return p; }
        p++;
        *cur = json_array_at_const(*cur, idx);
        return p;
    }

    // object key: everything up to the next '.' or '[' or end.
    const char *start = p;
    while (*p && *p != '.' && *p != '[') p++;
    size_t klen = (size_t)(p - start);
    if (klen == 0) { *cur = NULL; return p; }

    // copy the key out so we can nul-terminate for json_object_get.
    char stackbuf[128];
    char *key = stackbuf;
    char *heap = NULL;
    if (klen >= sizeof stackbuf) {
        heap = (char *)malloc(klen + 1);
        if (!heap) { *cur = NULL; return p; }
        key = heap;
    }
    memcpy(key, start, klen);
    key[klen] = 0;

    *cur = json_object_get_const(*cur, key);
    free(heap);
    return p;
}

const json_value *json_query(const json_value *root, const char *path) {
    if (!root || !path) return NULL;
    const json_value *cur = root;
    const char *p = path;

    // a leading dot is tolerated (so ".foo" == "foo"); makes generated paths
    // less fussy to assemble.
    if (*p == '.') p++;

    while (*p && cur) {
        if (*p == '.') { p++; continue; }
        p = step(&cur, p);
        if (!cur) return NULL;
    }
    return cur;
}

int json_get_bool(const json_value *root, const char *path, int fallback) {
    return json_as_bool(json_query(root, path), fallback);
}

double json_get_number(const json_value *root, const char *path, double fallback) {
    return json_as_number(json_query(root, path), fallback);
}

long long json_get_int(const json_value *root, const char *path, long long fallback) {
    return json_as_int(json_query(root, path), fallback);
}

const char *json_get_string(const json_value *root, const char *path, const char *fallback) {
    return json_as_string(json_query(root, path), fallback);
}
