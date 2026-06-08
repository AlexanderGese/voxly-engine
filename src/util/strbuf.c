#include "strbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strbuf_init(strbuf *s) {
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

void strbuf_free(strbuf *s) {
    free(s->data);
    s->data = NULL;
    s->len = s->cap = 0;
}

void strbuf_clear(strbuf *s) {
    if (s->data) s->data[0] = 0;
    s->len = 0;
}

void strbuf_reserve(strbuf *s, size_t n) {
    if (s->cap >= n + 1) return;
    size_t nc = s->cap ? s->cap : 16;
    while (nc < n + 1) nc *= 2;
    s->data = realloc(s->data, nc);
    s->cap = nc;
    if (s->len == 0) s->data[0] = 0;
}

void strbuf_append_n(strbuf *s, const char *str, size_t n) {
    strbuf_reserve(s, s->len + n);
    memcpy(s->data + s->len, str, n);
    s->len += n;
    s->data[s->len] = 0;
}

void strbuf_append(strbuf *s, const char *str) {
    strbuf_append_n(s, str, strlen(str));
}

void strbuf_append_char(strbuf *s, char c) {
    strbuf_reserve(s, s->len + 1);
    s->data[s->len++] = c;
    s->data[s->len] = 0;
}

void strbuf_vappendf(strbuf *s, const char *fmt, va_list ap) {
    va_list cp;
    va_copy(cp, ap);
    int n = vsnprintf(NULL, 0, fmt, cp);
    va_end(cp);
    if (n <= 0) return;
    strbuf_reserve(s, s->len + (size_t)n);
    vsnprintf(s->data + s->len, (size_t)n + 1, fmt, ap);
    s->len += (size_t)n;
}

void strbuf_appendf(strbuf *s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    strbuf_vappendf(s, fmt, ap);
    va_end(ap);
}
