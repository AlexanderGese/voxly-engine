#include "chat.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void chat_init(chatlog *c) {
    memset(c, 0, sizeof *c);
}

void chat_push(chatlog *c, const char *msg) {
    int i = c->head;
    strncpy(c->lines[i], msg, CHAT_LINE_LEN - 1);
    c->lines[i][CHAT_LINE_LEN - 1] = 0;
    c->ages[i] = 0;
    c->head = (c->head + 1) % CHAT_MAX_LINES;
    if (c->count < CHAT_MAX_LINES) c->count++;
}

void chat_pushf(chatlog *c, const char *fmt, ...) {
    char buf[CHAT_LINE_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    chat_push(c, buf);
}

void chat_update(chatlog *c, float dt) {
    for (int i = 0; i < CHAT_MAX_LINES; i++) c->ages[i] += dt;
}

void chat_draw(const chatlog *c, text_renderer *t, int sw, int sh) {
    int shown = 0;
    int y = sh - 120;
    for (int k = 0; k < c->count; k++) {
        int idx = (c->head - 1 - k + CHAT_MAX_LINES) % CHAT_MAX_LINES;
        if (c->ages[idx] > 10.0f) continue;
        text_draw(t, c->lines[idx], 8, y - shown * 10, 1, 1, 1, sw, sh);
        shown++;
        if (shown >= 8) break;
    }
}
