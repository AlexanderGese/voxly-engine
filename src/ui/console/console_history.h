#ifndef UI_CONSOLE_HISTORY_H
#define UI_CONSOLE_HISTORY_H
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
#endif
