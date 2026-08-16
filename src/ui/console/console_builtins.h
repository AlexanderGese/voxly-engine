#ifndef UI_CONSOLE_BUILTINS_H
#define UI_CONSOLE_BUILTINS_H

// the standard command + cvar set. registers everything onto a freshly
// initialised console_t. split out from console.c so the core stays free
// of game knowledge (block ids, player flags, the lot) and so adding a
// command is a one-liner in one obvious place.

#include "console.h"

void console_register_builtins(console_t *c);

#endif
