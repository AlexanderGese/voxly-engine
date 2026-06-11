#include "json_value.h"
#include "json_array.h"
#include "json_object.h"

#include <stdlib.h>
#include <string.h>

#include "../darray.h"

// small helper: strdup but length-bounded and tolerant of NULL.
static char *dup_n(const char *s, size_t n) {
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    if (n && s) memcpy(out, s, n);
    out[n] = 0;
    return out;
}

json_value json_null(void) {
    json_value v;
    memset(&v, 0, sizeof v);
    v.kind = JSON_NULL;
    return v;
}

json_value json_bool(int b) {
    json_value v = json_null();
    v.kind = JSON_BOOL;
    v.as.boolean = b ? 1 : 0;
    return v;
}

json_value json_number(double n) {
    json_value v = json_null();
    v.kind = JSON_NUMBER;
    v.as.number = n;
    v.is_int = 0;
    return v;
}

json_value json_int(long long n) {
    json_value v = json_null();
    v.kind = JSON_NUMBER;
    v.as.number = (double)n;
    v.is_int = 1;
    return v;
}

json_value json_string_n(const char *s, size_t n) {
    json_value v = json_null();
    char *copy = dup_n(s, n);
    if (!copy) return v;  // stays null on oom
    v.kind = JSON_STRING;
    v.as.str = copy;
    return v;
}

json_value json_string(const char *s) {
    return json_string_n(s, s ? strlen(s) : 0);
}

void json_free(json_value *v) {
    if (!v) return;
    switch (v->kind) {
        case JSON_STRING:
            free(v->as.str);
            break;
        case JSON_ARRAY: {
            size_t n = darr_len(v->as.arr);
            for (size_t i = 0; i < n; i++) json_free(&v->as.arr[i]);
            darr_free(v->as.arr);
            break;
        }
        case JSON_OBJECT: {
            size_t n = darr_len(v->as.obj);
            for (size_t i = 0; i < n; i++) {
                free(v->as.obj[i].key);
                json_free(&v->as.obj[i].val);
            }
            darr_free(v->as.obj);
            break;
        }
        default:
            break;  // null/bool/number own nothing
    }
    memset(v, 0, sizeof *v);
    v->kind = JSON_NULL;
}

json_value json_clone(const json_value *v) {
    if (!v) return json_null();
    switch (v->kind) {
        case JSON_NULL:   return json_null();
        case JSON_BOOL:   return json_bool(v->as.boolean);
        case JSON_STRING: return json_string(v->as.str);
        case JSON_NUMBER: {
            json_value c = json_number(v->as.number);
            c.is_int = v->is_int;
            return c;
        }
        case JSON_ARRAY: {
            json_value out = json_array();
            size_t n = darr_len(v->as.arr);
            for (size_t i = 0; i < n; i++) {
                json_value child = json_clone(&v->as.arr[i]);
                json_array_push(&out, child);
            }
            return out;
        }
        case JSON_OBJECT: {
            json_value out = json_object();
            size_t n = darr_len(v->as.obj);
            for (size_t i = 0; i < n; i++) {
                json_value child = json_clone(&v->as.obj[i].val);
                json_object_set(&out, v->as.obj[i].key, child);
            }
            return out;
        }
    }
    return json_null();
}

int json_as_bool(const json_value *v, int fallback) {
    if (!v) return fallback;
    if (v->kind == JSON_BOOL)   return v->as.boolean;
    // a number coerces the C way: nonzero is true. mildly evil but convenient.
    if (v->kind == JSON_NUMBER) return v->as.number != 0.0;
    return fallback;
}

double json_as_number(const json_value *v, double fallback) {
    if (v && v->kind == JSON_NUMBER) return v->as.number;
    return fallback;
}

long long json_as_int(const json_value *v, long long fallback) {
    if (v && v->kind == JSON_NUMBER) return (long long)v->as.number;
    return fallback;
}

const char *json_as_string(const json_value *v, const char *fallback) {
    if (v && v->kind == JSON_STRING) return v->as.str;
    return fallback;
}
