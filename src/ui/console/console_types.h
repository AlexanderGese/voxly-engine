#ifndef UI_CONSOLE_TYPES_H
#define UI_CONSOLE_TYPES_H

// shared types for the debug console subsystem. this is the new full
// fat console (input line editing, history, scrollback, command
// dispatch + cvars). the old src/ui/console.c is the one-line toy thing
// that just does tp/give/fly. didnt want to rip it out so this lives in
// its own dir and uses console_t (with the _t) to not collide.

#include <stdint.h>
#include <stddef.h>

// max chars in the input edit line. matches the toy console's 128 so
// muscle memory stays the same.
#define CONSOLE_LINE_LEN     256

// scrollback. ring of output lines.
#define CONSOLE_SCROLLBACK   256
#define CONSOLE_TEXT_LEN     200

// command history depth.
#define CONSOLE_HISTORY      64

// max tokens a single command line can lex into.
#define CONSOLE_MAX_TOKENS   32

// registries are fixed size, no point growing these at runtime.
#define CONSOLE_MAX_COMMANDS 128
#define CONSOLE_MAX_CVARS    128

// per-line severity. drives the text colour in uiconsole_draw.
typedef enum {
    CONSOLE_SEV_ECHO = 0,   // user typed it, greyish
    CONSOLE_SEV_INFO,       // normal output, white
    CONSOLE_SEV_WARN,       // yellow
    CONSOLE_SEV_ERROR,      // red
    CONSOLE_SEV_OK          // green, "did the thing"
} console_severity;

// rgb triple. could use vec3 but text_draw takes 3 floats and i didnt
// want to drag math/ into the header for three numbers.
typedef struct {
    float r, g, b;
} console_rgb;

console_rgb console_severity_color(console_severity sev);

#endif
