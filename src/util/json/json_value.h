#ifndef UTIL_JSON_VALUE_H
#define UTIL_JSON_VALUE_H
#include "json_types.h"
json_value json_null(void);
json_value json_bool(int b);
json_value json_number(double n);
json_value json_int(long long n);
json_value json_string(const char *s);
json_value json_string_n(const char *s, size_t n);
void json_free(json_value *v);
json_value json_clone(const json_value *v);
static inline int json_is_null  (const json_value *v) { return v && v->kind == JSON_NULL;   }
static inline int json_is_bool  (const json_value *v) { return v && v->kind == JSON_BOOL;
}
static inline int json_is_number(const json_value *v) { return v && v->kind == JSON_NUMBER; }
static inline int json_is_string(const json_value *v) { return v && v->kind == JSON_STRING;
}

// leaf accessors with a default. these never fault: wrong-kind or NULL returns
// the supplied fallback. that's deliberate - reading config shouldnt need a
// type check on every line.
int          json_as_bool  (const json_value *v, int fallback);
double       json_as_number(const json_value *v, double fallback);
long long    json_as_int   (const json_value *v, long long fallback);
const char  *json_as_string(const json_value *v, const char *fallback);
#endif
