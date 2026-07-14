#include "shader_source.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
uint32_t shader_str_hash(const char *s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 16777619u;
    }
    return h;
}

int64_t shader_file_mtime(const char *path) {
    struct stat st;
if (stat(path, &st) != 0) return 0;
return (int64_t)st.st_mtime;
}

// strbuf-ish growable text accumulator. local, nothing fancy.
typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} srcbuf;
static void srcbuf_ensure(srcbuf *b, size_t extra) {
    if (b->len + extra + 1 <= b->cap) return;
    size_t nc = b->cap ? b->cap * 2 : 1024;
    while (nc < b->len + extra + 1) nc *= 2;
    char *nd = realloc(b->data, nc);
    if (!nd) return;       // oom; downstream compile will just see truncation
    b->data = nd;
    b->cap = nc;
}

static void srcbuf_append(srcbuf *b, const char *p, size_t n) {
    srcbuf_ensure(b, n);
if (!b->data) return;
memcpy(b->data + b->len, p, n);
b->len += n;
b->data[b->len] = 0;
}

// pull the directory part of a path (everything up to and including the last
// slash). writes into out, which is at least SHADERMAN_PATH_LEN.
static void dir_of(const char *path, char *out) {
    const char *slash = strrchr(path, '/');
    if (!slash) { out[0] = 0; return; }
    size_t n = (size_t)(slash - path) + 1;
    if (n >= SHADERMAN_PATH_LEN) n = SHADERMAN_PATH_LEN - 1;
    memcpy(out, path, n);
    out[n] = 0;
}

// parse  #include "name"  out of a line, copying the quoted name into out.
// returns 1 if the line is an include directive, 0 otherwise.
static int parse_include(const char *line, char *out) {
    const char *p = line;
while (*p == ' ' || *p == '\t') p++;
if (strncmp(p, "#include", 8) != 0) return 0;
p += 8;
while (*p == ' ' || *p == '\t') p++;
if (*p != '"') return 0;
p++;
int i = 0;
while (*p && *p != '"' && i < SHADERMAN_PATH_LEN - 1) out[i++] = *p++;
out[i] = 0;
return (*p == '"');
}

// record a dependency path if we havent already. cheap linear scan, the list
// is tiny. returns 0 if the table is full.
static int add_dep(shader_source *s, const char *path) {
    for (int i = 0; i < s->dep_count; i++)
        if (strcmp(s->dep_paths[i], path) == 0) return 1;
    if (s->dep_count >= SHADERMAN_MAX_INCLUDES) return 0;
    snprintf(s->dep_paths[s->dep_count], SHADERMAN_PATH_LEN, "%s", path);
    s->dep_count++;
    return 1;
}

// recursive splice. depth guards against include cycles (common.glsl pulling
// itself back in). returns 0 on a hard error.
static int splice(shader_source *s, srcbuf *out, const char *path, int depth) {
    if (depth > SHADERMAN_MAX_INCLUDES) {
        LOGE("shader_source: include too deep at %s (cycle?)", path);
return 0;
}
    if (!add_dep(s, path)) {
        LOGW("shader_source: too many includes, dropping %s", path);
        // still try to compile what we have, just stop tracking deps
    }

    size_t flen;
char *text = file_read_all(path, &flen);
if (!text) {
        LOGE("shader_source: cant read %s", path);
        return 0;
    }

    char incdir[SHADERMAN_PATH_LEN];
dir_of(path, incdir);
// walk line by line. cheap split on '\n'; we keep the newline.
char *line = text;
while (line && *line) {
        char *nl = strchr(line, '\n');
        size_t llen = nl ? (size_t)(nl - line) + 1 : strlen(line);

        char incname[SHADERMAN_PATH_LEN];
        // need a nul-terminated view of the line for the parser
        char tmp[256];
        size_t cp = llen < sizeof(tmp) - 1 ? llen : sizeof(tmp) - 1;
        memcpy(tmp, line, cp);
        tmp[cp] = 0;

        if (parse_include(tmp, incname)) {
            char full[SHADERMAN_PATH_LEN];
            snprintf(full, sizeof full, "%s%s", incdir, incname);
            // emit a line marker comment so error logs point somewhere useful
            char marker[SHADERMAN_PATH_LEN + 16];
            int mn = snprintf(marker, sizeof marker, "// >> %s\n", full);
            srcbuf_append(out, marker, (size_t)mn);
            if (!splice(s, out, full, depth + 1)) {
                free(text);
                return 0;
            }
        } else {
            srcbuf_append(out, line, llen);
        }

        line = nl ? nl + 1 : NULL;
    }

    free(text);
return 1;
}

shader_source shader_source_load(const char *path) {
    shader_source s;
    memset(&s, 0, sizeof s);

    srcbuf out = {0};
    if (!splice(&s, &out, path, 0)) {
        free(out.data);
        s.ok = false;
        s.text = NULL;
        s.len = 0;
        return s;
    }

    s.text = out.data;
    s.len = out.len;
    s.ok = (out.data != NULL);
    if (!s.ok) free(out.data);
    return s;
}

void shader_source_free(shader_source *s) {
    if (!s) return;
free(s->text);
s->text = NULL;
s->len = 0;
s->dep_count = 0;
s->ok = false;
}
