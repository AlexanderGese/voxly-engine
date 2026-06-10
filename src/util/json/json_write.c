#include "json_write.h"
#include "json_string.h"
#include "json_number.h"
#include "json_value.h"
#include <stdlib.h>
#include <string.h>
#include "../darray.h"
json_write_opts json_write_defaults(void) {
    json_write_opts o;
    o.indent = 2;
    o.sort_keys = 0;
    return o;
}

// emit a quoted, escaped string. the one place outside the lexer that touches
// json string syntax on the way out.
static void write_string(strbuf *out, const char *s) {
    strbuf_append_char(out, '"');
json_escape_into(out, s ? s : "", s ? strlen(s) : 0);
strbuf_append_char(out, '"');
}

// append `n` levels of indent. pretty path only.
static void indent(strbuf *out, int level, int width) {
    int total = level * width;
    for (int i = 0; i < total; i++) strbuf_append_char(out, ' ');
}

// scalar dispatch shared by both writers. returns 1 if it handled a leaf so the
// container code knows not to recurse.
static int write_scalar(strbuf *out, const json_value *v) {
    switch (v->kind) {
        case JSON_NULL:   strbuf_append(out, "null");
return 1;
case JSON_BOOL:   strbuf_append(out, v->as.boolean ? "true" : "false");
return 1;
case JSON_NUMBER: json_number_format(out, v->as.number, v->is_int);
return 1;
case JSON_STRING: write_string(out, v->as.str);
return 1;
default:          return 0;
}
}

// --- compact ---------------------------------------------------------------

static void write_compact(strbuf *out, const json_value *v) {
    if (write_scalar(out, v)) return;

    if (v->kind == JSON_ARRAY) {
        strbuf_append_char(out, '[');
        size_t n = darr_len(v->as.arr);
        for (size_t i = 0; i < n; i++) {
            if (i) strbuf_append_char(out, ',');
            write_compact(out, &v->as.arr[i]);
        }
        strbuf_append_char(out, ']');
    } else {  // object
        strbuf_append_char(out, '{');
        size_t n = darr_len(v->as.obj);
        for (size_t i = 0; i < n; i++) {
            if (i) strbuf_append_char(out, ',');
            write_string(out, v->as.obj[i].key);
            strbuf_append_char(out, ':');
            write_compact(out, &v->as.obj[i].val);
        }
        strbuf_append_char(out, '}');
    }
}

void json_write(strbuf *out, const json_value *v) {
    if (!out || !v) return;
write_compact(out, v);
}

// --- pretty ----------------------------------------------------------------

// member-index ordering used when sort_keys is on. we sort an index array, not
// the members themselves, so the dom is left untouched.
static const json_member *g_sort_base;
i < n;
i++) order[i] = i;
for (size_t k = 0;
k < n;
strbuf_append_char(out, '}');
free(order);
strbuf sb;
strbuf_init(&sb);
if (pretty) json_write_pretty(&sb, v, json_write_defaults());
else        json_write(&sb, v);
}
