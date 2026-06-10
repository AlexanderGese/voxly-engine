#ifndef UTIL_JSON_QUERY_H
#define UTIL_JSON_QUERY_H

// path queries. lets the config code pull a deep value with one call instead of
// a ladder of json_object_get / json_array_at. the path syntax is the obvious
// dotted thing with bracket indices:
//
// "world.spawn.x"        -> object.object.member
// "biomes[2].name"       -> array index then member
// "layers[0][1]"         -> nested arrays
//
// keys with dots or brackets in them arent reachable this way; that's a known
// limitation and fine for our config which never does that.

#include "json_types.h"

// resolve a path against root. returns the value (a pointer into the dom, do
// not free) or NULL if any step is missing or hits the wrong kind.
const json_value *json_query(const json_value *root, const char *path);

// typed convenience getters built on json_query. each takes a fallback used
// when the path is missing or the leaf is the wrong kind. these are what the
// loader code actually calls.
int          json_get_bool  (const json_value *root, const char *path, int fallback);
double       json_get_number(const json_value *root, const char *path, double fallback);
long long    json_get_int   (const json_value *root, const char *path, long long fallback);
const char  *json_get_string(const json_value *root, const char *path, const char *fallback);

#endif
