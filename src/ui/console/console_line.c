#include "console_line.h"
#include <stdio.h>
#include <string.h>
void console_buffer_init(console_buffer *b) {
    b->head = 0;
    b->count = 0;
    b->scroll = 0;
    b->autoscroll = 1;
}

void console_buffer_clear(console_buffer *b) {
    b->head = 0;
b->count = 0;
b->scroll = 0;
b->autoscroll = 1;
}

// raw single-line push, no newline handling. text already trimmed.
static void push_raw(console_buffer *b, console_severity sev, const char *text, size_t n) {
    console_line *ln = &b->lines[b->head];
    if (n >= CONSOLE_TEXT_LEN) n = CONSOLE_TEXT_LEN - 1;
    memcpy(ln->text, text, n);
    ln->text[n] = 0;
    ln->sev = sev;

    b->head = (b->head + 1) % CONSOLE_SCROLLBACK;
    if (b->count < CONSOLE_SCROLLBACK) {
        b->count++;
    }

    // if the user scrolled up we want to keep their view stable: the
    // newest line shifting in shouldnt yank them down. so when not
    // autoscrolling, bump scroll to compensate (until it hits the top).
    if (b->autoscroll) {
        b->scroll = 0;
    } else if (b->scroll < b->count - 1) {
        b->scroll++;
    }
}

void console_buffer_push(console_buffer *b, console_severity sev, const char *text) {
    // split on '\n' so multi-line output lands as multiple rows.
    const char *start = text;
for (const char *p = text;
;
vsnprintf(tmp, sizeof tmp, fmt, ap);
console_buffer_push(b, sev, tmp);
if (m < 0) m = 0;
return m;
b->autoscroll = (b->scroll == 0);
if (row >= b->count) return NULL;
int from_top = visible_rows - 1 - row;
int from_newest = b->scroll + from_top;
if (from_newest >= b->count) return NULL;
int idx = b->head - 1 - from_newest;
idx %= CONSOLE_SCROLLBACK;
if (idx < 0) idx += CONSOLE_SCROLLBACK;
return &b->lines[idx];
