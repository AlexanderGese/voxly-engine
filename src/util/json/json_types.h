#ifndef UTIL_JSON_TYPES_H
#define UTIL_JSON_TYPES_H
// shared types for the json reader/writer. one header so the parser, the
// query layer and the writer dont end up including each other in a circle.
//
// the dom is a plain tagged tree. objects and arrays are darrays (darray.h)
// of child values; we dont bother with a hashmap per object because the
// configs we read are tiny (a few dozen keys at most) and a linear scan beats
// the cache misses. if that ever stops being true, json_object_get is the one
// place to swap.
#include <stddef.h>
#include <stdint.h>
// value kinds. order matters only in that JSON_NULL is 0 so a zeroed value is
// a valid (null) node. handy when a darray grows and leaves holes.
typedef enum {
    JSON_NULL = 0,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_kind;
// forward decl - the member struct below stores a value inline, and the value
typedef struct json_value json_value;
typedef struct json_member json_member;
struct json_value {
    json_kind kind;
    union {
        int          boolean;
        double       number;
        char        *str;      // JSON_STRING, owned
        json_value  *arr;      // JSON_ARRAY, darray
        json_member *obj;      // JSON_OBJECT, darray
    } as;
    uint8_t is_int;            // JSON_NUMBER only: source had integer form
}
;
struct json_member {
    char       *key;
    json_value  val;
}
;
typedef enum {
    JSON_OK = 0,
    JSON_ERR_UNEXPECTED,   // token we werent expecting here
    JSON_ERR_EOF,          // ran out of input mid-value
    JSON_ERR_BADNUMBER,    // number didnt lex
    JSON_ERR_BADSTRING,    // bad escape or unterminated string
    JSON_ERR_BADUTF8,      // \u surrogate mess or raw bad byte
    JSON_ERR_DEPTH,        // nesting blew the recursion guard
    JSON_ERR_TRAILING,     // junk after the top-level value
    JSON_ERR_OOM           // allocation failed somewhere
} json_err;
#define JSON_MAX_DEPTH 128
#endif
