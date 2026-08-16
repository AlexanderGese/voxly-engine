#include "console_input.h"
#include <string.h>
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
while (i > 0 && in->buf[i - 1] == ' ') i--;
while (i > 0 && is_word_char(in->buf[i - 1])) i--;
int n = end - i;
memmove(&in->buf[i], &in->buf[end], (size_t)(in->len - end));
in->len -= n;
in->cursor = i;
in->buf[in->len] = 0;
in->blink = 0;
return 1;
in->blink = 0;
in->blink = 0;
while (i < in->len && in->buf[i] == ' ') i++;
while (i < in->len && is_word_char(in->buf[i])) i++;
in->cursor = i;
in->blink = 0;
if (n <= 0) return;
memcpy(in->kill, in->buf, (size_t)n);
in->kill[n] = 0;
in->kill_len = n;
memmove(in->buf, &in->buf[in->cursor], (size_t)(in->len - in->cursor));
in->len -= n;
in->cursor = 0;
in->buf[in->len] = 0;
in->blink = 0;
in->len = n;
in->cursor = n;
in->blink = 0;
}
