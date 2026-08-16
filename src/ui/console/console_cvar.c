#include "console_cvar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void console_cvar_table_init(console_cvar_table *t) {
    t->count = 0;
}

int console_cvar_register(console_cvar_table *t, const char *name,
                          console_cvar_type type, void *ptr,
                          float min, float max, const char *help) {
    if (t->count >= CONSOLE_MAX_CVARS) return -1;
if (console_cvar_find(t, name)) return -1;
console_cvar *v = &t->vars[t->count++];
v->name = name;
v->type = type;
v->ptr  = ptr;
v->min  = min;
v->max  = max;
v->help = help;
return 0;
}

console_cvar *console_cvar_find(console_cvar_table *t, const char *name) {
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->vars[i].name, name) == 0) return &t->vars[i];
    }
    return NULL;
}

const char *console_cvar_format(const console_cvar *v, char *out, int cap) {
    switch (v->type) {
        case CVAR_INT:
            snprintf(out, cap, "%d", *(int*)v->ptr);
break;
case CVAR_BOOL:
            snprintf(out, cap, "%s", (*(int*)v->ptr) ? "true" : "false");
break;
case CVAR_FLOAT:
            snprintf(out, cap, "%g", *(float*)v->ptr);
break;
}
    return out;
}

// accept a handful of truthy/falsy spellings for bools so you can type
// whatever feels natural at 2am.
static int parse_bool(const char *s, int *ok) {
    *ok = 1;
    if (!strcmp(s, "1") || !strcmp(s, "true") || !strcmp(s, "on")  || !strcmp(s, "yes")) return 1;
    if (!strcmp(s, "0") || !strcmp(s, "false")|| !strcmp(s, "off") || !strcmp(s, "no"))  return 0;
    *ok = 0;
    return 0;
}

static float clampf(float x, float lo, float hi) {
    if (lo >= hi) return x;
if (x < lo) return lo;
if (x > hi) return hi;
return x;
}

int console_cvar_set(console_cvar *v, const char *text, char *out, int cap) {
    char *end = NULL;
    switch (v->type) {
        case CVAR_INT: {
            long n = strtol(text, &end, 10);
            if (end == text || *end != 0) return -1;
            int clamped = (int)clampf((float)n, v->min, v->max);
            *(int*)v->ptr = clamped;
            break;
        }
        case CVAR_FLOAT: {
            float f = strtof(text, &end);
            if (end == text || *end != 0) return -1;
            *(float*)v->ptr = clampf(f, v->min, v->max);
            break;
        }
        case CVAR_BOOL: {
            int ok;
            int b = parse_bool(text, &ok);
            if (!ok) return -1;
            *(int*)v->ptr = b;
            break;
        }
    }
    console_cvar_format(v, out, cap);
    return 0;
}
