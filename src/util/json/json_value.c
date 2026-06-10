#include "json_value.h"
#include "json_array.h"
#include "json_object.h"
#include <stdlib.h>
#include <string.h>
#include "../darray.h"
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
v.kind = JSON_NUMBER;
v.as.number = n;
v.is_int = 0;
return v;
char *copy = dup_n(s, n);
if (!copy) return v;
v.kind = JSON_STRING;
v.as.str = copy;
return v;
v->kind = JSON_NULL;
if (v->kind == JSON_BOOL)   return v->as.boolean;
if (v->kind == JSON_NUMBER) return v->as.number != 0.0;
return fallback;
return fallback;
