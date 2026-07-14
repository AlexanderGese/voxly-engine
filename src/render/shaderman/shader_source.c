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
if (!b->data) return;
memcpy(b->data + b->len, p, n);
b->len += n;
b->data[b->len] = 0;
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
return 0;
char *text = file_read_all(path, &flen);
dir_of(path, incdir);
char *line = text;
return 1;
free(s->text);
s->text = NULL;
s->len = 0;
s->dep_count = 0;
s->ok = false;
}
