#ifndef UI_MENU_H
#define UI_MENU_H

#include "../render/text.h"

// very simple text-only pause menu. up/down to navigate, enter to select.

typedef enum {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_RESUME,
    MENU_ACTION_SAVE,
    MENU_ACTION_QUIT,
} menu_action;

typedef struct {
    int    visible;
    int    cursor;
} menu;

void        menu_init(menu *m);
void        menu_toggle(menu *m);
void        menu_key_up(menu *m);
void        menu_key_down(menu *m);
menu_action menu_key_enter(menu *m);
void        menu_draw(const menu *m, text_renderer *t, int sw, int sh);

#endif
