#include "menu.h"

static const char *items[] = { "RESUME", "SAVE", "QUIT" };
#define MENU_N 3

void menu_init(menu *m) {
    m->visible = 0;
    m->cursor = 0;
}

void menu_toggle(menu *m) { m->visible = !m->visible; }

void menu_key_up(menu *m) {
    if (!m->visible) return;
    m->cursor = (m->cursor + MENU_N - 1) % MENU_N;
}

void menu_key_down(menu *m) {
    if (!m->visible) return;
    m->cursor = (m->cursor + 1) % MENU_N;
}

menu_action menu_key_enter(menu *m) {
    if (!m->visible) return MENU_ACTION_NONE;
    switch (m->cursor) {
    case 0: return MENU_ACTION_RESUME;
    case 1: return MENU_ACTION_SAVE;
    case 2: return MENU_ACTION_QUIT;
    }
    return MENU_ACTION_NONE;
}

void menu_draw(const menu *m, text_renderer *t, int sw, int sh) {
    if (!m->visible) return;
    int cx = sw / 2;
    int cy = sh / 2 - 40;
    text_draw(t, "== PAUSED ==", cx - 40, cy - 30, 1, 1, 1, sw, sh);
    for (int i = 0; i < MENU_N; i++) {
        char buf[32];
        if (i == m->cursor) {
            buf[0] = '>'; buf[1] = ' ';
            int j = 0;
            while (items[i][j] && j < 20) { buf[2 + j] = items[i][j]; j++; }
            buf[2 + j] = 0;
        } else {
            int j = 0;
            while (items[i][j] && j < 22) { buf[j] = items[i][j]; j++; }
            buf[j] = 0;
        }
        text_draw(t, buf, cx - 40, cy + i * 14, 1, 1, 1, sw, sh);
    }
}
