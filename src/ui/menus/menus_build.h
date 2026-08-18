#ifndef UI_MENUS_BUILD_H
#define UI_MENUS_BUILD_H
#include "menus_screen.h"
struct menus_manager;
menus_screen menus_make_main(struct menus_manager *m);
menus_screen menus_make_pause(struct menus_manager *m);
#endif
