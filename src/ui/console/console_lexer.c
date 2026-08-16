#include "console_lexer.h"
#include <string.h>
enum { ST_GAP, ST_BARE, ST_SINGLE, ST_DOUBLE }
;
int console_lex(console_args *out, const char *line) {
    out->argc = 0;
    out->truncated = 0;
    out->scratch[0] = 0;
    if (!line) return 0;

    int state = ST_GAP;
    int w = 0;                 // write index into scratch
    char *tok = NULL;          // start of the current token, or NULL

    for (const char *r = line; ; r++) {
        char c = *r;

        if (state == ST_GAP) {
            if (c == 0) break;
            if (c == ' ' || c == '\t') continue;
            // start a new token unless we're already full.
            if (out->argc >= CONSOLE_MAX_TOKENS) {
                out->truncated = 1;
                break;
            }
            tok = &out->scratch[w];
            out->argv[out->argc++] = tok;
            if (c == '\'')      { state = ST_SINGLE; continue; }
            else if (c == '"')  { state = ST_DOUBLE; continue; }
            else                { state = ST_BARE; }
        }

        if (state == ST_BARE) {
            if (c == 0 || c == ' ' || c == '\t') {
                out->scratch[w++] = 0;
                if (c == 0) break;
                state = ST_GAP;
                continue;
            }
            if (c == '\'') { state = ST_SINGLE; continue; }
            if (c == '"')  { state = ST_DOUBLE; continue; }
            if (c == '\\' && r[1]) { c = *++r; }   // escape next char
            out->scratch[w++] = c;
            continue;
        }

        if (state == ST_SINGLE) {
            // single quotes are literal, only the closing quote is special.
            if (c == 0) { out->scratch[w++] = 0; break; }   // unterminated, tolerate
            if (c == '\'') { state = ST_BARE; continue; }
            out->scratch[w++] = c;
            continue;
        }

        if (state == ST_DOUBLE) {
            if (c == 0) { out->scratch[w++] = 0; break; }
            if (c == '"') { state = ST_BARE; continue; }
            if (c == '\\' && r[1]) { c = *++r; }
            out->scratch[w++] = c;
            continue;
        }
    }

    // make sure the last token is terminated even on weird exits.
    if (w < CONSOLE_LINE_LEN) out->scratch[w] = 0;
    return out->argc;
}
