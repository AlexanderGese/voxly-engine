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
        s++; p++;
    }
    return 1;
}

void console_enter(console *c, world *w, player *p) {
    const char *cmd = c->input;

    if (starts_with(cmd, "tp ")) {
        float x, y, z;
        if (sscanf(cmd + 3, "%f %f %f", &x, &y, &z) == 3) {
            p->pos = (vec3){x, y, z};
            snprintf(c->last_output, sizeof c->last_output, "tp to %.1f %.1f %.1f", x, y, z);
        }
    } else if (starts_with(cmd, "give ")) {
        int id = atoi(cmd + 5);
        if (id >= 0 && id < BLOCK_COUNT) {
            snprintf(c->last_output, sizeof c->last_output, "gave %s", block_get(id)->name);
        } else {
            snprintf(c->last_output, sizeof c->last_output, "unknown block id");
        }
    } else if (starts_with(cmd, "seed")) {
        snprintf(c->last_output, sizeof c->last_output, "seed=%u", w->seed);
    } else if (starts_with(cmd, "fly")) {
        p->flying = !p->flying;
        snprintf(c->last_output, sizeof c->last_output, "fly=%d", p->flying);
    } else if (starts_with(cmd, "help") || cmd[0] == 0) {
        snprintf(c->last_output, sizeof c->last_output,
                 "cmds: tp x y z | give id | seed | fly | help");
    } else {
        snprintf(c->last_output, sizeof c->last_output, "unknown cmd");
    }

    c->input_len = 0;
    c->input[0] = 0;
    c->visible = 0;
}

void console_draw(const console *c, text_renderer *t, int sw, int sh) {
    if (!c->visible && c->last_output[0] == 0) return;
    int y = sh - 24;
    if (c->visible) {
        char buf[CONSOLE_LINE_LEN + 4];
        snprintf(buf, sizeof buf, "> %s_", c->input);
        text_draw(t, buf, 8, y, 1, 1, 0.6f, sw, sh);
    }
    if (c->last_output[0]) {
        text_draw(t, c->last_output, 8, y - 14, 0.8f, 0.8f, 1.0f, sw, sh);
    }
}
