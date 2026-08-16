#ifndef UI_CONSOLE_CONSOLE_H
#define UI_CONSOLE_CONSOLE_H
#include "console_types.h"
#include "console_line.h"
#include "console_input.h"
#include "console_history.h"
#include "console_command.h"
#include "console_cvar.h"
#include <stdarg.h>
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
void console_bind(console_t *c, struct world *w, struct player *p);
void console_toggle(console_t *c);
void console_set_open(console_t *c, int open);
int  console_is_open(const console_t *c);
void console_tick(console_t *c, float dt);
#endif
