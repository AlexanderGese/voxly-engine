#ifndef UI_CONSOLE_HISTORY_H
#define UI_CONSOLE_HISTORY_H

// command history. a ring of the last N entered lines + an up/down
// cursor for recall. shell-ish: consecutive duplicate lines collapse,
// and when you start recalling we stash whatever you were typing so
// pressing down past the newest restores it.

#include "console_types.h"

typedef struct {
    char entries[CONSOLE_HISTORY][CONSOLE_LINE_LEN];
    int  head;        // next write slot
    int  count;       // valid entries

    // recall cursor. -1 means "not recalling, editing fresh".
    int  cursor;      // 0 == most recent, grows older
    char stash[CONSOLE_LINE_LEN];   // the unsubmitted line we set aside
    int  has_stash;
} console_history;

void console_history_init(console_history *h);

// record a submitted command. ignores empties and exact repeats of the
// most recent entry. also resets the recall cursor.
void console_history_add(console_history *h, const char *line);

// begin/continue recall. pass the line currently in the edit box so it
// can be stashed on the first step. writes the recalled text into out
// (cap bytes) and returns 1 if it moved, 0 if already at an end.
int  console_history_prev(console_history *h, const char *current, char *out, int cap);
int  console_history_next(console_history *h, char *out, int cap);

// drop the recall cursor (e.g. after the user edits the line).
void console_history_reset(console_history *h);

int  console_history_count(const console_history *h);

#endif
