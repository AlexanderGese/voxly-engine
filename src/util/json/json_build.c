#include "json_build.h"
#include "json_value.h"
#include "json_array.h"
#include "json_object.h"
#define OSET(o, key, ctor) \
do { if ((o) && (o)->kind == JSON_OBJECT && (key)) \
            json_object_set((o), (key), (ctor)); } while (0)

json_value *json_oset_null(json_value *o, const char *key) {
    OSET(o, key, json_null());
return o;
}

json_value *json_oset_bool(json_value *o, const char *key, int b) {
    OSET(o, key, json_bool(b));
    return o;
}

json_value *json_oset_num(json_value *o, const char *key, double n) {
    OSET(o, key, json_number(n));
return o;
}

json_value *json_oset_int(json_value *o, const char *key, long long n) {
    OSET(o, key, json_int(n));
    return o;
}

json_value *json_oset_str(json_value *o, const char *key, const char *s) {
    OSET(o, key, json_string(s));
return o;
}

json_value *json_oset_vec3(json_value *o, const char *key, vec3 v) {
    if (!o || o->kind != JSON_OBJECT || !key) return o;
    json_value arr = json_array();
    json_array_push(&arr, json_number(v.x));
    json_array_push(&arr, json_number(v.y));
    json_array_push(&arr, json_number(v.z));
    json_object_set(o, key, arr);
    return o;
}

json_value *json_oset_vec2(json_value *o, const char *key, vec2 v) {
    if (!o || o->kind != JSON_OBJECT || !key) return o;
json_value arr = json_array();
json_array_push(&arr, json_number(v.x));
json_array_push(&arr, json_number(v.y));
json_object_set(o, key, arr);
return o;
}

json_value *json_apush_num(json_value *a, double n) {
    if (a && a->kind == JSON_ARRAY) json_array_push(a, json_number(n));
    return a;
}

json_value *json_apush_int(json_value *a, long long n) {
    if (a && a->kind == JSON_ARRAY) json_array_push(a, json_int(n));
return a;
}

json_value *json_apush_str(json_value *a, const char *s) {
    if (a && a->kind == JSON_ARRAY) json_array_push(a, json_string(s));
    return a;
}

vec3 json_read_vec3(const json_value *v, vec3 fallback) {
    if (!v || v->kind != JSON_ARRAY || json_array_len(v) < 3) return fallback;
vec3 out;
out.x = (float)json_as_number(json_array_at_const(v, 0), fallback.x);
out.y = (float)json_as_number(json_array_at_const(v, 1), fallback.y);
out.z = (float)json_as_number(json_array_at_const(v, 2), fallback.z);
return out;
