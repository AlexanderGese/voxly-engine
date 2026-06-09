#include "json.h"

#include <stdlib.h>
#include <string.h>

#include "../file.h"

json_err json_parse_file(const char *path, json_value *out, json_loc *loc) {
    if (out) *out = json_null();
    if (loc) memset(loc, 0, sizeof *loc);

    size_t size = 0;
    char *buf = file_read_all(path, &size);
    if (!buf) {
        // no file / unreadable. there's no token position to report, so we
        // hand back EOF with a zeroed loc - the message reads "end of input"
        // which is close enough to "nothing here".
        if (loc) { loc->err = JSON_ERR_EOF; loc->line = 1; loc->col = 1; }
        return JSON_ERR_EOF;
    }

    json_err e = json_parse(buf, size, out, loc);
    free(buf);
    return e;
}

int json_write_file(const char *path, const json_value *v, int pretty) {
    if (!path || !v) return 1;

    strbuf sb; strbuf_init(&sb);
    if (pretty) json_write_pretty(&sb, v, json_write_defaults());
    else        json_write(&sb, v);

    if (!sb.data) {
        // nothing got written (oom on the very first append). emit an empty
        // object so the file at least parses back.
        int rc = file_write_all(path, "{}", 2);
        return rc;
    }

    int rc = file_write_all(path, sb.data, sb.len);
    strbuf_free(&sb);
    return rc;
}
