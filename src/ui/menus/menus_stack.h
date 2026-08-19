#ifndef UI_MENUS_STACK_H
#define UI_MENUS_STACK_H
// the screen stack. push/pop/replace, with the enter/leave callbacks fired at the
// right edges. the manager drives the *top* screen each frame; everything below
// is dimmed but kept alive so a "back" restores its scroll/focus exactly.
//
// fixed depth — menus never nest more than a few deep (main -> settings ->
// keybinds is the worst case). a fixed array means no allocs on navigation, which
// matters because we push/pop while the user mashes escape.
#include "menus_screen.h"
#define MENUS_STACK_MAX 8
struct menus_manager;
// fwd
typedef struct {
    menus_screen screens[MENUS_STACK_MAX];
    int          top;       // index of the active screen, -1 when empty
} menus_stack;
void menus_stack_init(menus_stack *st);
// pop everything, firing leave(destroyed) on each. used when the whole menu
// system closes (resume to world).
void menus_stack_clear(menus_stack *st, struct menus_manager *m);
int  menus_stack_empty(const menus_stack *st);
int  menus_stack_depth(const menus_stack *st);
// the active screen, or NULL when empty.
menus_screen *menus_stack_top(menus_stack *st);
// push a freshly-initialized screen. the previous top gets leave(destroyed=0),
// the new one gets enter(). returns the pushed slot, or NULL if the stack is
// full (in which case the screen's owned state is freed to avoid a leak).
menus_screen *menus_stack_push(menus_stack *st, struct menus_manager *m,
                               const menus_screen *src);
// pop the top, firing leave(destroyed=1) and freeing owned state, then enter() on
// whatever's revealed underneath. no-op on an empty stack.
void menus_stack_pop(menus_stack *st, struct menus_manager *m);
// replace the top in place (leave-destroyed old, enter new). same as pop+push but
// keeps depth constant — used by the manager when it swaps main->pause without a
// back path between them.
menus_screen *menus_stack_replace(menus_stack *st, struct menus_manager *m,
                                  const menus_screen *src);
#endif
