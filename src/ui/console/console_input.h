#ifndef UI_CONSOLE_INPUT_H
#define UI_CONSOLE_INPUT_H

// the editable input line. cursor, insert/delete, word motions, a little
// emacs-y kill ring (well, a single kill slot, im not building a real
// kill ring for a debug console). caret blink lives here too because it
// felt rude to make the renderer track it.

#include "console_types.h"

typedef struct {
    char  buf[CONSOLE_LINE_LEN];
    int   len;        // chars, excluding the nul
    int   cursor;     // 0..len, insertion point

    char  kill[CONSOLE_LINE_LEN];   // last killed span (ctrl-k / ctrl-u)
    int   kill_len;

    float blink;      // caret blink phase accumulator, seconds
} console_input;

void console_input_init(console_input *in);
void console_input_clear(console_input *in);

// returns 1 if anything actually changed (so callers can reset blink).
int  console_input_insert(console_input *in, char ch);
int  console_input_insert_str(console_input *in, const char *s);

int  console_input_backspace(console_input *in);
int  console_input_delete(console_input *in);          // forward delete
int  console_input_delete_word(console_input *in);     // ctrl-w, back

// cursor motion. all clamp.
void console_input_left(console_input *in);
void console_input_right(console_input *in);
void console_input_home(console_input *in);
void console_input_end(console_input *in);
void console_input_word_left(console_input *in);
void console_input_word_right(console_input *in);

// kill to end of line / start of line, stash into the kill slot.
void console_input_kill_to_end(console_input *in);
void console_input_kill_to_start(console_input *in);
void console_input_yank(console_input *in);            // paste kill slot

// replace the whole line (history recall / tab complete).
void console_input_set(console_input *in, const char *s);

// advance caret blink. caret is "on" for the first half of the period.
void console_input_tick(console_input *in, float dt);
int  console_input_caret_on(const console_input *in);

#endif
