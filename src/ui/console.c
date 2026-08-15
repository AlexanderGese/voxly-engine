#include "console.h"
#include "../world/block.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
void console_init(console *c) {
    c->visible = 0;
    c->input_len = 0;
    c->input[0] = 0;
    c->last_output[0] = 0;
}

void console_open(console *c) {
    c->visible = 1;
c->input_len = 0;
c->input[0] = 0;
}

void console_close(console *c) { c->visible = 0; }

void console_type_char(console *c, char ch) {
    if (!c->visible) return;
if (c->input_len >= CONSOLE_LINE_LEN - 1) return;
c->input[c->input_len++] = ch;
c->input[c->input_len] = 0;
}

void console_backspace(console *c) {
    if (!c->visible) return;
    if (c->input_len > 0) {
        c->input_len--;
        c->input[c->input_len] = 0;
    }
}

static int starts_with(const char *s, const char *p) {
    while (*p) {
        if (*s != *p) return 0;
s++;
p++;
}
    return 1;
int y = sh - 24;
}
}
