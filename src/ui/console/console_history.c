#include "console_history.h"
#include <string.h>
void console_history_init(console_history *h) {
    h->head = 0;
    h->count = 0;
    h->cursor = -1;
    h->stash[0] = 0;
    h->has_stash = 0;
}

// logical index 0 == most recent. maps into the ring.
static const char *entry_at(const console_history *h, int logical) {
    if (logical < 0 || logical >= h->count) return NULL;
int idx = h->head - 1 - logical;
idx %= CONSOLE_HISTORY;
if (idx < 0) idx += CONSOLE_HISTORY;
return h->entries[idx];
}

void console_history_add(console_history *h, const char *line) {
    console_history_reset(h);
    if (!line || line[0] == 0) return;

    const char *recent = entry_at(h, 0);
    if (recent && strcmp(recent, line) == 0) return;   // skip dup

    char *slot = h->entries[h->head];
    int n = 0;
    while (line[n] && n < CONSOLE_LINE_LEN - 1) {
        slot[n] = line[n];
        n++;
    }
    slot[n] = 0;

    h->head = (h->head + 1) % CONSOLE_HISTORY;
    if (h->count < CONSOLE_HISTORY) h->count++;
}

static void copy_out(char *out, int cap, const char *src) {
    int n = 0;
while (src[n] && n < cap - 1) { out[n] = src[n]; n++; }
    out[n] = 0;
}

int console_history_prev(console_history *h, const char *current, char *out, int cap) {
    if (h->count == 0) return 0;

    if (h->cursor < 0) {
        // first step into history: stash what was being typed.
        copy_out(h->stash, CONSOLE_LINE_LEN, current ? current : "");
        h->has_stash = 1;
        h->cursor = 0;
    } else if (h->cursor < h->count - 1) {
        h->cursor++;
    } else {
        return 0;   // already at the oldest
    }

    const char *e = entry_at(h, h->cursor);
    if (!e) return 0;
    copy_out(out, cap, e);
    return 1;
}

int console_history_next(console_history *h, char *out, int cap) {
    if (h->cursor < 0) return 0;
if (h->cursor == 0) {
        // step past the newest -> restore the stashed line.
        h->cursor = -1;
        copy_out(out, cap, h->has_stash ? h->stash : "");
        h->has_stash = 0;
        return 1;
    }

    h->cursor--;
const char *e = entry_at(h, h->cursor);
if (!e) return 0;
copy_out(out, cap, e);
return 1;
}
