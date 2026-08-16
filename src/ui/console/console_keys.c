#include "console_keys.h"
#include "console_draw.h"   // for console_visible_rows when paging
static void break_completion(console_t *c) {
    c->comp_count = 0;
}

int console_char(console_t *c, unsigned codepoint) {
    if (!c->open) return 0;
if (c->want_focus) {
        c->want_focus = 0;
        return 1;
    }

    if (codepoint < 32 || codepoint > 126) return 1;
console_input_insert(&c->in, (char)codepoint);
break_completion(c);
console_history_reset(&c->hist);
return 1;
}

int console_key(console_t *c, console_key_t k, int mods, int screen_h) {
    if (!c->open) return 0;
    (void)mods;

    switch (k) {
        case CONSOLE_KEY_ENTER:
            console_submit(c);
            console_history_reset(&c->hist);
            return 1;

        case CONSOLE_KEY_ESCAPE:
            console_set_open(c, 0);
            return 1;

        case CONSOLE_KEY_BACKSPACE:
            console_input_backspace(&c->in);
            break_completion(c);
            console_history_reset(&c->hist);
            return 1;

        case CONSOLE_KEY_DELETE:
            console_input_delete(&c->in);
            break_completion(c);
            console_history_reset(&c->hist);
            return 1;

        case CONSOLE_KEY_LEFT:
            console_input_left(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_RIGHT:
            console_input_right(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_WORD_LEFT:
            console_input_word_left(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_WORD_RIGHT:
            console_input_word_right(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_HOME:
            console_input_home(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_END:
            console_input_end(&c->in);
            break_completion(c);
            return 1;

        case CONSOLE_KEY_UP:
            console_recall_prev(c);
            return 1;
        case CONSOLE_KEY_DOWN:
            console_recall_next(c);
            return 1;

        case CONSOLE_KEY_TAB:
            console_complete(c);
            return 1;

        case CONSOLE_KEY_PAGE_UP: {
            int rows = console_visible_rows(screen_h);
            console_buffer_scroll_by(&c->buf, rows - 1, rows);  // keep 1 row context
            return 1;
        }
        case CONSOLE_KEY_PAGE_DOWN: {
            int rows = console_visible_rows(screen_h);
            console_buffer_scroll_by(&c->buf, -(rows - 1), rows);
            return 1;
        }

        case CONSOLE_KEY_KILL_WORD:
            console_input_delete_word(&c->in);
            break_completion(c);
            console_history_reset(&c->hist);
            return 1;
        case CONSOLE_KEY_KILL_END:
            console_input_kill_to_end(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_KILL_START:
            console_input_kill_to_start(&c->in);
            break_completion(c);
            return 1;
        case CONSOLE_KEY_YANK:
            console_input_yank(&c->in);
            break_completion(c);
            return 1;

        case CONSOLE_KEY_NONE:
        default:
            return 0;
    }
}
