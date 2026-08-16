#include "console.h"
#include "console_builtins.h"
#include <stdio.h>
#include <string.h>
void console_t_init(console_t *c) {
    c->open = 0;
    c->want_focus = 0;

    console_buffer_init(&c->buf);
    console_input_init(&c->in);
    console_history_init(&c->hist);
    console_cmd_table_init(&c->cmds);
    console_cvar_table_init(&c->cvars);

    c->comp_count = 0;
    c->comp_index = 0;
    c->comp_stem[0] = 0;

    c->world  = NULL;
    c->player = NULL;

    c->last_line[0] = 0;
    c->last_sev = CONSOLE_SEV_INFO;

    // wire up the standard command set + engine cvars.
    console_register_builtins(c);

    // a little greeting so an empty console doesnt look broken.
    console_print(c, CONSOLE_SEV_INFO, "voxl console. type 'help'.");
}

void console_bind(console_t *c, struct world *w, struct player *p) {
    c->world  = w;
c->player = p;
}

void console_set_open(console_t *c, int open) {
    if (open && !c->open) {
        // opening: eat the toggle char and pin the view to the bottom.
        c->want_focus = 1;
        console_buffer_scroll_end(&c->buf);
        c->in.blink = 0;
    }
    c->open = open ? 1 : 0;
    // drop any half-finished completion cycle when visibility flips.
    c->comp_count = 0;
}

void console_toggle(console_t *c) {
    console_set_open(c, !c->open);
}

int console_is_open(const console_t *c) {
    return c->open;
}

void console_tick(console_t *c, float dt) {
    if (!c->open) return;
console_input_tick(&c->in, dt);
}

void console_vprintf(console_t *c, console_severity sev, const char *fmt, va_list ap) {
    // format once so we can both push it and keep a copy for last_line.
    char tmp[CONSOLE_TEXT_LEN * 2];
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    console_buffer_push(&c->buf, sev, tmp);

    // stash a trimmed single-line version for the closed-state hud peek.
    int n = 0;
    while (tmp[n] && tmp[n] != '\n' && n < CONSOLE_TEXT_LEN - 1) {
        c->last_line[n] = tmp[n];
        n++;
    }
    c->last_line[n] = 0;
    c->last_sev = sev;
}

void console_printf(console_t *c, console_severity sev, const char *fmt, ...) {
    va_list ap;
va_start(ap, fmt);
console_vprintf(c, sev, fmt, ap);
va_end(ap);
}

void console_print(console_t *c, console_severity sev, const char *text) {
    // route through printf so last_line bookkeeping stays in one place.
    console_printf(c, sev, "%s", text);
}

void console_submit(console_t *c) {
    // grab the line before we clear, history wants its own copy.
    char line[CONSOLE_LINE_LEN];
memcpy(line, c->in.buf, (size_t)(c->in.len + 1));
console_history_add(&c->hist, line);
console_dispatch(c, line);
console_input_clear(&c->in);
c->comp_count = 0;
console_buffer_scroll_end(&c->buf);
console_input_set(&c->in, c->comp[c->comp_index]);
return;
}

    grab_stem(&c->in, c->comp_stem, sizeof c->comp_stem);
char lcp[CONSOLE_LINE_LEN];
c->comp_count = console_cmd_complete(&c->cmds, c->comp_stem, c->comp,
                                         CONSOLE_MAX_COMMANDS, lcp, sizeof lcp);
c->comp_index = 0;
console_input_insert(&c->in, ' ');
c->comp_count = 0;
return;
char row[CONSOLE_TEXT_LEN];
int rl = 0;
row[0] = 0;
for (int i = 0;
i < c->comp_count;
}
