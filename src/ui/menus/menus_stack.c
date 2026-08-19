#include "menus_stack.h"

#include <stdlib.h>

void menus_stack_init(menus_stack *st) {
    st->top = -1;
}

int menus_stack_empty(const menus_stack *st) {
    return st->top < 0;
}

int menus_stack_depth(const menus_stack *st) {
    return st->top + 1;
}

menus_screen *menus_stack_top(menus_stack *st) {
    if (st->top < 0) return NULL;
    return &st->screens[st->top];
}

// free a screen's state if it owns it. centralized so push/pop/replace/clear all
// agree on ownership rules.
static void drop_state(menus_screen *s) {
    if (s->owns_state && s->state) {
        free(s->state);
        s->state = NULL;
    }
}

void menus_stack_clear(menus_stack *st, struct menus_manager *m) {
    while (st->top >= 0) {
        menus_screen *s = &st->screens[st->top];
        menus_screen_leave(m, s, 1);
        drop_state(s);
        st->top--;
    }
}

menus_screen *menus_stack_push(menus_stack *st, struct menus_manager *m,
                               const menus_screen *src) {
    if (st->top + 1 >= MENUS_STACK_MAX) {
        // full. don't silently leak the caller's owned state.
        menus_screen tmp = *src;
        drop_state(&tmp);
        return NULL;
    }

    menus_screen *old = menus_stack_top(st);
    if (old) menus_screen_leave(m, old, 0);

    st->top++;
    st->screens[st->top] = *src;
    menus_screen *now = &st->screens[st->top];
    menus_screen_enter(m, now);
    return now;
}

void menus_stack_pop(menus_stack *st, struct menus_manager *m) {
    if (st->top < 0) return;

    menus_screen *s = &st->screens[st->top];
    menus_screen_leave(m, s, 1);
    drop_state(s);
    st->top--;

    menus_screen *revealed = menus_stack_top(st);
    if (revealed) menus_screen_enter(m, revealed);
}

menus_screen *menus_stack_replace(menus_stack *st, struct menus_manager *m,
                                  const menus_screen *src) {
    if (st->top < 0) {
        // nothing to replace; degenerate to a push so callers don't special-case
        // the empty stack.
        return menus_stack_push(st, m, src);
    }

    menus_screen *s = &st->screens[st->top];
    menus_screen_leave(m, s, 1);
    drop_state(s);

    *s = *src;
    menus_screen_enter(m, s);
    return s;
}
