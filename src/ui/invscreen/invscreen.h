#ifndef UI_INVSCREEN_H
#define UI_INVSCREEN_H

// the inventory/crafting screen controller. this is the one thing the game pokes
// at: open/close it, feed it a frame of input, ask it to draw. it owns the model
// and wires together layout, drag, crafting and tooltips so the rest of the
// engine doesn't have to know any of those pieces exist.
//
// it does NOT own the widget context or the draw backend — the host passes a
// wg_draw_list in. that keeps it composable with the other hud panels which all
// share one draw list per frame.

#include "../widgets/widgets_draw.h"
#include "invscreen_model.h"
#include "invscreen_layout.h"
#include "invscreen_drag.h"
#include "invscreen_craft.h"
#include "invscreen_tooltip.h"

typedef struct {
    invscreen_model   model;
    invscreen_layout  layout;
    invscreen_drag    drag;
    invscreen_craft   craft;
    invscreen_tooltip tip;

    int   open;          // logically open (accepting input)
    float anim;          // 0..1 open progress, drives scale/fade
    int   sw, sh;        // last known screen size; layout rebuilt when it moves

    // a drop request the host should service: when the player throws a stack out
    // of the panel we stash it here and the host spawns a world item, then calls
    // invscreen_take_drop to clear it.
    int      drop_pending;
    block_id drop_block;
    int      drop_count;
} invscreen;

void invscreen_init(invscreen *iv);

// toggle / explicit open-close. opening doesn't reset the contents, just the
// transient interaction state (drag, tooltip). closing flushes any held stack
// back into the grid so items aren't stranded on the cursor.
void invscreen_open(invscreen *iv);
void invscreen_close(invscreen *iv);
void invscreen_toggle(invscreen *iv);
int  invscreen_is_open(const invscreen *iv);

// one frame of interaction. mouse position in screen px, button edges already
// resolved by the caller (1 = pressed/released this frame), shift latch for
// quick-move. `held_l` is whether the left button is currently down (for paint
// sweeps). dt advances animations + tooltip dwell.
void invscreen_update(invscreen *iv, int screen_w, int screen_h,
                      float mx, float my,
                      int left_pressed, int left_released, int left_held,
                      int right_pressed, int shift, float dt);

// record draw commands for this frame. no-op while fully closed.
void invscreen_render(invscreen *iv, wg_draw_list *dl, float mx, float my);

// host hands items in (pickups) and asks back for thrown stacks.
int  invscreen_give(invscreen *iv, block_id id, int amount); // returns leftover
int  invscreen_take_drop(invscreen *iv, block_id *id, int *amount);

#endif
