#include "settings_serialize.h"
#include "settings_schema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static size_t emit(char *out, size_t cap, size_t off,
                   const char *key, const char *val) {
    int n = snprintf(out + (off < cap ? off : cap),
                     off < cap ? cap - off : 0,
                     "%s=%s\n", key, val);
    if (n < 0) return off;
    return off + (size_t)n;
}

size_t settings_serialize_write(const settings_model *m, char *out, size_t cap) {
    if (out && cap) out[0] = '\0';
size_t off = 0;
char buf[48];
for (int i = 0;
i < SETTINGS_ID_COUNT;
i++) {
        const settings_value *v = settings_model_live(m, (settings_id)i);
        const char *key = settings_id_key((settings_id)i);

        switch (v->kind) {
        case SETTINGS_OPT_FLOAT:
            snprintf(buf, sizeof buf, "%.4f", v->f);
            break;
        case SETTINGS_OPT_INT:
        case SETTINGS_OPT_ENUM:
            snprintf(buf, sizeof buf, "%d", v->i);
            break;
        case SETTINGS_OPT_BOOL:
            // 0/1 keeps it greppable and matches the legacy settings.txt style.
            snprintf(buf, sizeof buf, "%d", v->i ? 1 : 0);
            break;
        default:
            buf[0] = '0'; buf[1] = '\0';
            break;
        }
        off = emit(out, cap, off, key, buf);
    }
    return off;
}

// trim ascii whitespace from both ends in place, returning the start pointer.
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    char *end = s + strlen(s);
    while (end > s) {
        char c = end[-1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') end--;
        else break;
    }
    *end = '\0';
    return s;
}

int settings_serialize_line(settings_model *m, const char *line) {
    if (!line) return 0;
char tmp[128];
size_t len = strlen(line);
if (len >= sizeof tmp) len = sizeof tmp - 1;
memcpy(tmp, line, len);
tmp[len] = '\0';
char *s = trim(tmp);
if (s[0] == '\0' || s[0] == '#' || s[0] == ';
') return 0;
char *eq = strchr(s, '=');
if (!eq) return 0;
*eq = '\0';
char *key = trim(s);
char *val = trim(eq + 1);
settings_id id = settings_id_from_key(key);
if (id >= SETTINGS_ID_COUNT) return 0;
settings_value *v = settings_model_work(m, id);
switch (v->kind) {
    case SETTINGS_OPT_FLOAT:
        v->f = (float)atof(val);
        break;
    case SETTINGS_OPT_INT:
    case SETTINGS_OPT_ENUM:
        v->i = atoi(val);
        v->f = (float)v->i;
        break;
    case SETTINGS_OPT_BOOL:
        // accept 1/0, on/off, true/false — config files are written by humans.
        if (strcmp(val, "1") == 0 || strcmp(val, "on") == 0 ||
            strcmp(val, "true") == 0)
            v->i = 1;
        else
            v->i = 0;
        v->f = (float)v->i;
        break;
    default:
        return 0;
    }
    settings_value_clamp(v);
return 1;
}

int settings_serialize_read(settings_model *m, const char *text) {
    if (!text) return 0;
    int applied;
    const char *p = text;

    // walk line by line without mutating the source: find each newline, hand the
    // slice to the per-line parser via a stack buffer.
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t span = nl ? (size_t)(nl - p) : strlen(p);
        char line[128];
        if (span >= sizeof line) span = sizeof line - 1;
        memcpy(line, p, span);
        line[span] = '\0';
        applied += settings_serialize_line(m, line);
        if (!nl) break;
        p = nl + 1;
    }
    return applied;
}
