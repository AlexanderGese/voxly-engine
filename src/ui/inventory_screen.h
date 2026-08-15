#ifndef UI_INVENTORY_SCREEN_H
#define UI_INVENTORY_SCREEN_H

#include "../player/inventory.h"
#include "../render/text.h"

// full inventory screen. shows all slots as text rows. open with 'i'.

typedef struct {
    int visible;
    int cursor;
} inventory_screen;

void inv_screen_init(inventory_screen *is);
void inv_screen_toggle(inventory_screen *is);
void inv_screen_up(inventory_screen *is);
void inv_screen_down(inventory_screen *is);
void inv_screen_draw(const inventory_screen *is, const inventory *inv,
                     text_renderer *t, int sw, int sh);

#endif
