#ifndef UTIL_JSON_ARRAY_H
#define UTIL_JSON_ARRAY_H

// array container ops. the array storage is a darray of json_value living in
// value->as.arr. these helpers exist so call sites never touch the darray
// macros directly and so json_array() can be the one place that stamps the kind.

#include "json_types.h"

// fresh empty array value. the backing darray is lazily allocated on first push.
json_value json_array(void);

// number of elements. 0 for a non-array (so loops over the wrong kind just
// dont run instead of faulting).
size_t json_array_len(const json_value *v);

// append. takes ownership of `child` - after this the array frees it. if `v`
// isnt an array this is a no-op and `child` is freed so nothing leaks.
void json_array_push(json_value *v, json_value child);

// element by index. returns NULL out of range or wrong kind. the pointer is
// into the backing store and is invalidated by any push that grows the array,
// so dont stash it across a push.
json_value       *json_array_at(json_value *v, size_t i);
const json_value *json_array_at_const(const json_value *v, size_t i);

// remove element i, shifting the tail down. frees the removed value. no-op if
// out of range. preserves order, which is what callers expect from json.
void json_array_remove(json_value *v, size_t i);

#endif
