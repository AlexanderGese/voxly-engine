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
    for (const char *p = text;; p++) {
        if (*p == '\n' || *p == 0) {
            push_raw(b, sev, start, (size_t)(p - start));
            if (*p == 0) break;
            start = p + 1;
        }
    }
}

void console_buffer_vpushf(console_buffer *b, console_severity sev, const char *fmt, va_list ap) {
    char tmp[CONSOLE_TEXT_LEN * 2];
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    console_buffer_push(b, sev, tmp);
}

void console_buffer_pushf(console_buffer *b, console_severity sev, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    console_buffer_vpushf(b, sev, fmt, ap);
    va_end(ap);
}

// highest scroll value that still shows content. you can scroll up until
// the oldest line sits at the top row of the window.
static int scroll_max(const console_buffer *b, int visible_rows) {
    int m = b->count - visible_rows;
    if (m < 0) m = 0;
    return m;
}

void console_buffer_scroll_by(console_buffer *b, int step, int visible_rows) {
    b->scroll += step;
    int m = scroll_max(b, visible_rows);
    if (b->scroll < 0) b->scroll = 0;
    if (b->scroll > m) b->scroll = m;
    // re-arm autoscroll only once we're parked back at the bottom.
    b->autoscroll = (b->scroll == 0);
}

void console_buffer_scroll_home(console_buffer *b, int visible_rows) {
    b->scroll = scroll_max(b, visible_rows);
    b->autoscroll = (b->scroll == 0);
}

void console_buffer_scroll_end(console_buffer *b) {
    b->scroll = 0;
    b->autoscroll = 1;
}

const console_line *console_buffer_visible(const console_buffer *b, int row, int visible_rows) {
    if (row < 0 || row >= visible_rows) return NULL;
    if (row >= b->count) return NULL;

    // newest line is at head-1. the bottom-most visible row is offset by
    // scroll from the newest. work out the logical index from the top of
    // the window downward.
    int from_top = visible_rows - 1 - row;          // 0 == bottom row
    int from_newest = b->scroll + from_top;
    if (from_newest >= b->count) return NULL;        // empty rows up top

    int idx = b->head - 1 - from_newest;
    idx %= CONSOLE_SCROLLBACK;
    if (idx < 0) idx += CONSOLE_SCROLLBACK;
    return &b->lines[idx];
}

int console_buffer_len(const console_buffer *b) {
    return b->count;
}
