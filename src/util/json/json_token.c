#include "json_token.h"
#include "json_number.h"

#include <string.h>

void json_lex_init(json_lexer *lx, const char *src, size_t len) {
    lx->src  = src;
    lx->len  = len;
    lx->pos  = 0;
    lx->line = 1;
    lx->col  = 1;
    lx->err  = JSON_OK;
}

// advance one byte, tracking line/col. newlines bump the line and reset col.
static char advance(json_lexer *lx) {
    char c = lx->src[lx->pos++];
    if (c == '\n') { lx->line++; lx->col = 1; }
    else           { lx->col++; }
    return c;
}

static void skip_ws(json_lexer *lx) {
    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        // strict json whitespace only: space, tab, lf, cr. no comments.
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') advance(lx);
        else break;
    }
}

// stamp a single-char punctuation token and step over it.
static json_token punct(json_lexer *lx, json_tok_kind k) {
    json_token t;
    t.kind  = k;
    t.begin = lx->src + lx->pos;
    t.len   = 1;
    t.line  = lx->line;
    t.col   = lx->col;
    advance(lx);
    return t;
}

static json_token make_error(json_lexer *lx, json_err e, int line, int col) {
    json_token t;
    t.kind  = JSON_TOK_ERROR;
    t.begin = lx->src + lx->pos;
    t.len   = 0;
    t.line  = line;
    t.col   = col;
    lx->err = e;
    return t;
}

// match a bare keyword (true/false/null) at the cursor. returns 1 and produces
// the token on a hit, else leaves the cursor put and returns 0.
static int keyword(json_lexer *lx, const char *kw, json_tok_kind k, json_token *out) {
    size_t kl = strlen(kw);
    if (lx->pos + kl > lx->len) return 0;
    if (memcmp(lx->src + lx->pos, kw, kl) != 0) return 0;
    out->kind  = k;
    out->begin = lx->src + lx->pos;
    out->len   = kl;
    out->line  = lx->line;
    out->col   = lx->col;
    for (size_t i = 0; i < kl; i++) advance(lx);
    return 1;
}

// scan a quoted string. the cursor sits on the opening quote. we record the
// span between the quotes and validate escapes only lightly here (enough to
// find the real closing quote); full validation happens at unescape time.
static json_token scan_string(json_lexer *lx) {
    int sline = lx->line, scol = lx->col;
    advance(lx);  // opening quote
    const char *body = lx->src + lx->pos;
    size_t start = lx->pos;

    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (c == '"') {
            json_token t;
            t.kind  = JSON_TOK_STRING;
            t.begin = body;
            t.len   = lx->pos - start;
            t.line  = sline;
            t.col   = scol;
            advance(lx);  // closing quote
            return t;
        }
        if (c == '\\') {
            // skip the escape introducer plus the escaped byte so a \" inside
            // the string doesnt look like the terminator.
            advance(lx);
            if (lx->pos >= lx->len) break;
            advance(lx);
            continue;
        }
        if ((unsigned char)c < 0x20) {
            return make_error(lx, JSON_ERR_BADSTRING, lx->line, lx->col);
        }
        advance(lx);
    }
    return make_error(lx, JSON_ERR_EOF, sline, scol);  // unterminated
}

static json_token scan_number(json_lexer *lx) {
    int sline = lx->line, scol = lx->col;
    double val; int isint;
    size_t got = json_number_scan(lx->src + lx->pos, lx->len - lx->pos, &val, &isint);
    if (got == 0) {
        return make_error(lx, JSON_ERR_BADNUMBER, sline, scol);
    }
    json_token t;
    t.kind  = JSON_TOK_NUMBER;
    t.begin = lx->src + lx->pos;
    t.len   = got;
    t.line  = sline;
    t.col   = scol;
    for (size_t i = 0; i < got; i++) advance(lx);
    return t;
}

json_token json_lex_next(json_lexer *lx) {
    skip_ws(lx);
    if (lx->pos >= lx->len) {
        json_token t;
        t.kind = JSON_TOK_EOF; t.begin = lx->src + lx->pos;
        t.len = 0; t.line = lx->line; t.col = lx->col;
        return t;
    }

    char c = lx->src[lx->pos];
    switch (c) {
        case '{': return punct(lx, JSON_TOK_LBRACE);
        case '}': return punct(lx, JSON_TOK_RBRACE);
        case '[': return punct(lx, JSON_TOK_LBRACKET);
        case ']': return punct(lx, JSON_TOK_RBRACKET);
        case ':': return punct(lx, JSON_TOK_COLON);
        case ',': return punct(lx, JSON_TOK_COMMA);
        case '"': return scan_string(lx);
        default: break;
    }

    json_token t;
    if (c == 't' && keyword(lx, "true",  JSON_TOK_TRUE,  &t)) return t;
    if (c == 'f' && keyword(lx, "false", JSON_TOK_FALSE, &t)) return t;
    if (c == 'n' && keyword(lx, "null",  JSON_TOK_NULL,  &t)) return t;

    if (c == '-' || (c >= '0' && c <= '9')) return scan_number(lx);

    return make_error(lx, JSON_ERR_UNEXPECTED, lx->line, lx->col);
}
