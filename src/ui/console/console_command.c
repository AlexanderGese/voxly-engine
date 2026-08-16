#include "console_command.h"
#include "console.h"

#include <string.h>

void console_cmd_table_init(console_cmd_table *t) {
    t->count = 0;
}

int console_cmd_register(console_cmd_table *t, const char *name,
                         console_cmd_fn fn, const char *usage, const char *help) {
    if (t->count >= CONSOLE_MAX_COMMANDS) return -1;
    if (console_cmd_find(t, name)) return -1;   // already taken

    console_command *c = &t->cmds[t->count++];
    c->name  = name;
    c->fn    = fn;
    c->usage = usage;
    c->help  = help;
    return 0;
}

console_command *console_cmd_find(console_cmd_table *t, const char *name) {
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->cmds[i].name, name) == 0) return &t->cmds[i];
    }
    return NULL;
}

int console_dispatch(struct console_t *c, const char *line) {
    // skip leading blanks; an all-whitespace line is a no-op.
    while (*line == ' ' || *line == '\t') line++;
    if (line[0] == 0) return -1;

    // echo the command back so the scrollback reads like a transcript.
    console_printf(c, CONSOLE_SEV_ECHO, "] %s", line);

    console_args args;
    console_lex(&args, line);
    if (args.argc == 0) return -1;
    if (args.truncated) {
        console_print(c, CONSOLE_SEV_WARN, "too many tokens, line truncated");
    }

    console_command *cmd = console_cmd_find(&c->cmds, args.argv[0]);
    if (!cmd) {
        console_printf(c, CONSOLE_SEV_ERROR,
                       "unknown command '%s'. try 'help'.", args.argv[0]);
        return -1;
    }

    console_cmd_result r = cmd->fn(c, &args);
    if (r == CONSOLE_CMD_USAGE) {
        console_printf(c, CONSOLE_SEV_WARN, "usage: %s",
                       cmd->usage ? cmd->usage : cmd->name);
    }
    // ERR handlers print their own gripe, nothing to do here.
    return 0;
}

// shared prefix length of two strings, in bytes.
static int common_prefix(const char *a, const char *b) {
    int n = 0;
    while (a[n] && a[n] == b[n]) n++;
    return n;
}

int console_cmd_complete(const console_cmd_table *t, const char *prefix,
                         const char **names, int cap, char *lcp, int lcp_cap) {
    int plen = (int)strlen(prefix);
    int found = 0;
    const char *first = NULL;
    int lcp_len = 0;

    for (int i = 0; i < t->count; i++) {
        const char *nm = t->cmds[i].name;
        if (strncmp(nm, prefix, (size_t)plen) != 0) continue;

        if (found < cap) names[found] = nm;

        if (first == NULL) {
            first = nm;
            lcp_len = (int)strlen(nm);
        } else {
            // shrink the running longest-common-prefix against this match.
            int k = common_prefix(first, nm);
            if (k < lcp_len) lcp_len = k;
        }
        found++;
    }

    // export the lcp so the caller can extend the edit line to it.
    if (lcp) {
        if (first == NULL) {
            lcp[0] = 0;
        } else {
            if (lcp_len >= lcp_cap) lcp_len = lcp_cap - 1;
            memcpy(lcp, first, (size_t)lcp_len);
            lcp[lcp_len] = 0;
        }
    }
    return found;
}
