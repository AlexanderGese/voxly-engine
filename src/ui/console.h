#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "../render/text.h"
#include "../world/world.h"
#include "../player/player.h"

// developer console. / opens it. type a command, enter runs it.

#define CONSOLE_LINE_LEN 128

typedef struct {
    int  visible;
    char input[CONSOLE_LINE_LEN];
    int  input_len;
    char last_output[256];
} console;

void console_init(console *c);
void console_open(console *c);
void console_close(console *c);
void console_type_char(console *c, char ch);
void console_backspace(console *c);
void console_enter(console *c, world *w, player *p);
void console_draw(const console *c, text_renderer *t, int sw, int sh);

#endif
