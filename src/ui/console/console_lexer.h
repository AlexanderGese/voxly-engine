#ifndef UI_CONSOLE_LEXER_H
#define UI_CONSOLE_LEXER_H

// splits a command line into argv-style tokens. handles single/double
// quotes and backslash escapes, collapses runs of whitespace. tokens
// point into a caller-owned scratch buffer (we mangle a copy in place,
// classic argv-from-a-string trick).

#include "console_types.h"

typedef struct {
    char  scratch[CONSOLE_LINE_LEN];     // chewed-up copy of the line
    char *argv[CONSOLE_MAX_TOKENS];      // point into scratch
    int   argc;
    int   truncated;                     // hit CONSOLE_MAX_TOKENS
} console_args;

// lex `line` into `out`. returns argc. safe on NULL/empty (argc 0).
int console_lex(console_args *out, const char *line);

#endif
