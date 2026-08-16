#ifndef UI_CONSOLE_CVAR_H
#define UI_CONSOLE_CVAR_H

// console variables. quake-style. a named knob that lives somewhere in
// the engine; the console can read and poke it via `set name value` and
// `get name`. we dont own the storage, we just hold a pointer + a type
// tag and do the parse/format. keeps the gameplay code free of console
// awareness.

#include "console_types.h"

typedef enum {
    CVAR_INT,
    CVAR_FLOAT,
    CVAR_BOOL
} console_cvar_type;

typedef struct {
    const char        *name;
    console_cvar_type  type;
    void              *ptr;        // -> int / float / int(bool)
    float              min, max;   // clamp range; min>=max disables clamp
    const char        *help;
} console_cvar;

typedef struct {
    console_cvar vars[CONSOLE_MAX_CVARS];
    int          count;
} console_cvar_table;

void console_cvar_table_init(console_cvar_table *t);

// register a cvar. returns 0 ok, -1 if full or name clashes.
int  console_cvar_register(console_cvar_table *t, const char *name,
                           console_cvar_type type, void *ptr,
                           float min, float max, const char *help);

console_cvar *console_cvar_find(console_cvar_table *t, const char *name);

// format the current value into out. returns out for convenience.
const char *console_cvar_format(const console_cvar *v, char *out, int cap);

// parse text into the cvar's storage, clamping. returns 0 ok, -1 on a
// bad number. on success writes the stored-back value into out.
int  console_cvar_set(console_cvar *v, const char *text, char *out, int cap);

#endif
