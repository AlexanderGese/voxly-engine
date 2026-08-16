#ifndef UI_CONSOLE_CONSOLE_H
#define UI_CONSOLE_CONSOLE_H

// the fat debug console. owns the scrollback, the edit line, command +
// cvar tables and the recall history, and wires keystrokes through to all
// of them. the engine pokes world/player pointers in here so the builtin
// commands can actually do something. drawing lives in uiconsole_draw.c.
//
// header lives at src/ui/console/console.h, distinct from the old toy
// src/ui/console.h, and the struct is console_t (the _t) to keep the two
// from colliding in any TU that happens to drag both in.

#include "console_types.h"
#include "console_line.h"
#include "console_input.h"
#include "console_history.h"
#include "console_command.h"
#include "console_cvar.h"

#include <stdarg.h>

// fwd decls for the game-side pointers. we never deref these in core, only
// in the builtin command handlers, so a forward is enough here.
struct world;
struct player;

typedef struct console_t {
    int  open;          // panel showing + eating input
    int  want_focus;    // set the frame we opened, so the toggle key char
                        // ('/' or '`') doesnt leak into the edit line

    console_buffer   buf;
    console_input    in;
    console_history  hist;
    console_cmd_table  cmds;
    console_cvar_table cvars;

    // the live completion candidate list, rebuilt each time tab cycles.
    const char *comp[CONSOLE_MAX_COMMANDS];
    int         comp_count;
    int         comp_index;     // which candidate we're parked on
    char        comp_stem[CONSOLE_LINE_LEN];   // the word we're completing

    // game hooks. builtins reach through these. left NULL = commands that
    // need them print a polite "no world loaded".
    struct world  *world;
    struct player *player;

    // tiny aggregate so the hud can show a one-line status when closed.
    char last_line[CONSOLE_TEXT_LEN];
    console_severity last_sev;
} console_t;

void console_t_init(console_t *c);

// hand the console its game-side pointers (call after the world loads).
void console_bind(console_t *c, struct world *w, struct player *p);

void console_toggle(console_t *c);
void console_set_open(console_t *c, int open);
int  console_is_open(const console_t *c);

// per-frame: advances the caret blink. dt in seconds.
void console_tick(console_t *c, float dt);

// printing helpers. these are what command handlers and the rest of the
// engine call. they fan out into the scrollback and stash last_line.
void console_print(console_t *c, console_severity sev, const char *text);
void console_printf(console_t *c, console_severity sev, const char *fmt, ...);
void console_vprintf(console_t *c, console_severity sev, const char *fmt, va_list ap);

// submit the current edit line: dispatch it, record history, clear input.
void console_submit(console_t *c);

// tab completion driver. first press extends to the longest common prefix
// (and lists candidates if ambiguous); repeats cycle through candidates.
void console_complete(console_t *c);

// recall. wraps the history module + pushes the result into the editor.
void console_recall_prev(console_t *c);
void console_recall_next(console_t *c);

#endif
