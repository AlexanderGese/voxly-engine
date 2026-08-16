#ifndef UI_CONSOLE_KEYS_H
#define UI_CONSOLE_KEYS_H

// the key-event front end. the game loop already decodes GLFW into edges;
// here we translate those edges + typed codepoints into edits, recall,
// completion, scrolling and submit. kept apart from console.c so the core
// stays input-agnostic and unit-testable without GLFW in the room.

#include "console.h"

// semantic keys the console cares about. the caller maps its raw GLFW
// codes onto these before calling console_key. this indirection means the
// console never #includes glfw and the keybinds file stays the one source
// of truth for actual scancodes.
typedef enum {
    CONSOLE_KEY_NONE = 0,
    CONSOLE_KEY_ENTER,
    CONSOLE_KEY_BACKSPACE,
    CONSOLE_KEY_DELETE,
    CONSOLE_KEY_LEFT,
    CONSOLE_KEY_RIGHT,
    CONSOLE_KEY_HOME,
    CONSOLE_KEY_END,
    CONSOLE_KEY_UP,         // history prev
    CONSOLE_KEY_DOWN,       // history next
    CONSOLE_KEY_TAB,        // complete
    CONSOLE_KEY_PAGE_UP,    // scroll back a screenful
    CONSOLE_KEY_PAGE_DOWN,
    CONSOLE_KEY_ESCAPE,     // close
    CONSOLE_KEY_WORD_LEFT,  // ctrl-left
    CONSOLE_KEY_WORD_RIGHT, // ctrl-right
    CONSOLE_KEY_KILL_WORD,  // ctrl-w
    CONSOLE_KEY_KILL_END,   // ctrl-k
    CONSOLE_KEY_KILL_START, // ctrl-u
    CONSOLE_KEY_YANK        // ctrl-y
} console_key_t;

// modifier bits, or together. only ctrl actually changes behaviour today.
enum {
    CONSOLE_MOD_SHIFT = 1 << 0,
    CONSOLE_MOD_CTRL  = 1 << 1,
    CONSOLE_MOD_ALT   = 1 << 2
};

// the panel must be open and focused for these to do anything. screen_h is
// needed so paging knows the window's row count. returns 1 if the event
// was consumed (so the caller can stop it reaching the game).
int console_key(console_t *c, console_key_t k, int mods, int screen_h);

// a typed character (already filtered to printable ascii by the platform
// text-input callback). ignored on the frame we opened, so the toggle key
// doesnt land in the line. returns 1 if consumed.
int console_char(console_t *c, unsigned codepoint);

#endif
