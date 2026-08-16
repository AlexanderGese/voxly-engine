#include "console_input.h"

#include <string.h>

// caret blink period in seconds. on for the first half.
#define BLINK_PERIOD 1.0f

void console_input_init(console_input *in) {
    memset(in, 0, sizeof *in);
}

void console_input_clear(console_input *in) {
    in->buf[0] = 0;
    in->len = 0;
    in->cursor = 0;
    in->blink = 0;
}

static int is_word_char(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '_';
}

int console_input_insert(console_input *in, char ch) {
    if (in->len >= CONSOLE_LINE_LEN - 1) return 0;
    // shift the tail right by one to open a gap at the cursor.
    memmove(&in->buf[in->cursor + 1], &in->buf[in->cursor],
            (size_t)(in->len - in->cursor));
    in->buf[in->cursor] = ch;
    in->len++;
    in->cursor++;
    in->buf[in->len] = 0;
    in->blink = 0;
    return 1;
}

int console_input_insert_str(console_input *in, const char *s) {
    int any = 0;
    for (; *s; s++) {
        // drop control chars that sneak in via paste.
        if (*s == '\n' || *s == '\r' || *s == '\t') continue;
        if (!console_input_insert(in, *s)) break;
        any = 1;
    }
    return any;
}

int console_input_backspace(console_input *in) {
    if (in->cursor == 0) return 0;
    memmove(&in->buf[in->cursor - 1], &in->buf[in->cursor],
            (size_t)(in->len - in->cursor));
    in->len--;
    in->cursor--;
    in->buf[in->len] = 0;
    in->blink = 0;
    return 1;
}

int console_input_delete(console_input *in) {
    if (in->cursor >= in->len) return 0;
    memmove(&in->buf[in->cursor], &in->buf[in->cursor + 1],
            (size_t)(in->len - in->cursor - 1));
    in->len--;
    in->buf[in->len] = 0;
    in->blink = 0;
    return 1;
}

int console_input_delete_word(console_input *in) {
    if (in->cursor == 0) return 0;
    int end = in->cursor;
    int i = in->cursor;
    // eat trailing spaces first, then the word.
    while (i > 0 && in->buf[i - 1] == ' ') i--;
    while (i > 0 && is_word_char(in->buf[i - 1])) i--;
    int n = end - i;
    memmove(&in->buf[i], &in->buf[end], (size_t)(in->len - end));
    in->len -= n;
    in->cursor = i;
    in->buf[in->len] = 0;
    in->blink = 0;
    return 1;
}

void console_input_left(console_input *in) {
    if (in->cursor > 0) in->cursor--;
    in->blink = 0;
}

void console_input_right(console_input *in) {
    if (in->cursor < in->len) in->cursor++;
    in->blink = 0;
}

void console_input_home(console_input *in) { in->cursor = 0; in->blink = 0; }
void console_input_end(console_input *in)  { in->cursor = in->len; in->blink = 0; }

void console_input_word_left(console_input *in) {
    int i = in->cursor;
    while (i > 0 && in->buf[i - 1] == ' ') i--;
    while (i > 0 && is_word_char(in->buf[i - 1])) i--;
    in->cursor = i;
    in->blink = 0;
}

void console_input_word_right(console_input *in) {
    int i = in->cursor;
    while (i < in->len && in->buf[i] == ' ') i++;
    while (i < in->len && is_word_char(in->buf[i])) i++;
    in->cursor = i;
    in->blink = 0;
}

void console_input_kill_to_end(console_input *in) {
    int n = in->len - in->cursor;
    if (n <= 0) return;
    memcpy(in->kill, &in->buf[in->cursor], (size_t)n);
    in->kill[n] = 0;
    in->kill_len = n;
    in->len = in->cursor;
    in->buf[in->len] = 0;
    in->blink = 0;
}

void console_input_kill_to_start(console_input *in) {
    int n = in->cursor;
    if (n <= 0) return;
    memcpy(in->kill, in->buf, (size_t)n);
    in->kill[n] = 0;
    in->kill_len = n;
    memmove(in->buf, &in->buf[in->cursor], (size_t)(in->len - in->cursor));
    in->len -= n;
    in->cursor = 0;
    in->buf[in->len] = 0;
    in->blink = 0;
}

void console_input_yank(console_input *in) {
    if (in->kill_len <= 0) return;
    console_input_insert_str(in, in->kill);
}

void console_input_set(console_input *in, const char *s) {
    int n = 0;
    while (s[n] && n < CONSOLE_LINE_LEN - 1) {
        in->buf[n] = s[n];
        n++;
    }
    in->buf[n] = 0;
    in->len = n;
    in->cursor = n;
    in->blink = 0;
}

void console_input_tick(console_input *in, float dt) {
    in->blink += dt;
    // wrap so the float doesnt slowly lose precision over a long session.
    while (in->blink >= BLINK_PERIOD) in->blink -= BLINK_PERIOD;
}

int console_input_caret_on(const console_input *in) {
    return in->blink < BLINK_PERIOD * 0.5f;
}
