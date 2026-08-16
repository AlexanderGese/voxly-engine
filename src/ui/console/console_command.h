#ifndef UI_CONSOLE_COMMAND_H
#define UI_CONSOLE_COMMAND_H
// command registry + dispatch. a command is a name, a handler, and some
// help text. handlers get the already-lexed argv and a console_t* so
// they can print back into the scrollback. the world/player pointers ride
// along on the console_t (see console.h) so commands can poke the game.
#include "console_types.h"
#include "console_lexer.h"
struct console_t;
// forward; full def in console.h
// handler return is just a status for the dispatcher's bookkeeping.
typedef enum {
    CONSOLE_CMD_OK = 0,
    CONSOLE_CMD_ERR,         // command ran but failed (bad args etc.)
    CONSOLE_CMD_USAGE        // arg shape was wrong, print usage
} console_cmd_result;
typedef console_cmd_result (*console_cmd_fn)(struct console_t *c,
                                             const console_args *args);
typedef struct {
    const char     *name;
    console_cmd_fn  fn;
    const char     *usage;   // e.g. "tp <x> <y> <z>"
    const char     *help;    // one-line description
} console_command;
typedef struct {
    console_command cmds[CONSOLE_MAX_COMMANDS];
    int             count;
} console_cmd_table;
void console_cmd_table_init(console_cmd_table *t);
int  console_cmd_register(console_cmd_table *t, const char *name,
                          console_cmd_fn fn, const char *usage, const char *help);
console_command *console_cmd_find(console_cmd_table *t, const char *name);
// lex `line`, find the command, run it. prints echo + errors into the
// console buffer. returns 0 if a command ran, -1 if not found / empty.
int  console_dispatch(struct console_t *c, const char *line);
// prefix match over command names, for tab completion. fills names[] with
// up to cap pointers, returns how many matched. also reports the longest
// common prefix into lcp (cap bytes) so the caller can extend the input.
int  console_cmd_complete(const console_cmd_table *t, const char *prefix,
                          const char **names, int cap, char *lcp, int lcp_cap);
#endif
