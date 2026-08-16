#ifndef UI_CONSOLE_LINE_H
#define UI_CONSOLE_LINE_H

// scrollback buffer. a ring of fixed-width lines + a severity per line.
// also owns the scroll offset (how far up from the bottom we're looking)
// and the autoscroll latch.

#include "console_types.h"
#include <stdarg.h>

typedef struct {
    char             text[CONSOLE_TEXT_LEN];
    console_severity sev;
} console_line;

typedef struct {
    console_line lines[CONSOLE_SCROLLBACK];
    int          head;      // index of next write slot
    int          count;     // how many valid lines (<= CONSOLE_SCROLLBACK)

    // view state. scroll is "lines scrolled up from the newest". 0 means
    // pinned to the bottom. autoscroll re-pins on every push.
    int          scroll;
    int          autoscroll;
} console_buffer;

void console_buffer_init(console_buffer *b);
void console_buffer_clear(console_buffer *b);

// push one line. truncates to CONSOLE_TEXT_LEN-1. embedded newlines get
// split into separate lines so callers dont have to care.
void console_buffer_push(console_buffer *b, console_severity sev, const char *text);
void console_buffer_pushf(console_buffer *b, console_severity sev, const char *fmt, ...);
void console_buffer_vpushf(console_buffer *b, console_severity sev, const char *fmt, va_list ap);

// scroll controls. clamp internally. step is in lines (can be negative).
void console_buffer_scroll_by(console_buffer *b, int step, int visible_rows);
void console_buffer_scroll_home(console_buffer *b, int visible_rows);
void console_buffer_scroll_end(console_buffer *b);

// fetch the i-th visible line counting from the top of the current view.
// returns NULL once you run off the end. visible_rows is how many rows
// the window can show. used by the renderer.
const console_line *console_buffer_visible(const console_buffer *b, int row, int visible_rows);

// total lines currently held.
int console_buffer_len(const console_buffer *b);

#endif
