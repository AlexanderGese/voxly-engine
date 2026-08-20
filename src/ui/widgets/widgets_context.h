#ifndef UI_WIDGETS_CONTEXT_H
#define UI_WIDGETS_CONTEXT_H

// the immediate-mode context. this is the spine everything else hangs off.
//
// the classic muratori-style hot/active model:
// - "hot"    = the widget the mouse is over and could interact with
// - "active" = the widget currently being held / dragged
// each widget gets a stable id (a hash of a caller string + an int salt) so we
// can carry that one bit of state across frames without storing per-widget
// structs. everything else about a widget is recomputed from scratch every
// frame — that's the whole point of immediate mode.

#include "widgets_types.h"
#include "widgets_input.h"
#include "widgets_draw.h"
#include "widgets_style.h"

typedef uint32_t wg_id;

// id stack depth. panels push their id so child widgets get namespaced; you'd
// have to nest panels ~32 deep to overflow this and at that point the ui is the
// bug, not this array.
#define WG_ID_STACK 32

typedef struct {
    wg_input     input;
    wg_draw_list draw;
    wg_style     style;

    float screen_w, screen_h;
    float dt;                  // seconds since last ui frame, for animations

    // interaction state, persists across frames
    wg_id hot;                 // candidate this frame
    wg_id hot_prev;            // candidate from last frame (for first-hover anim)
    wg_id active;              // held/dragged
    wg_id focus;               // keyboard focus (tab target)

    // value scratch for the active widget. sliders stash the grab offset here so
    // dragging feels anchored instead of snapping the handle to the cursor.
    float active_v0;
    float active_mouse0;

    // id namespacing stack
    wg_id id_stack[WG_ID_STACK];
    int   id_top;

    // did anything claim the mouse this frame. the host checks this to decide
    // whether the click should also fall through to the game world.
    int   want_mouse;
    int   want_keys;
} wg_context;

void wg_init(wg_context *ctx);
void wg_free(wg_context *ctx);

// begin/end a ui frame. begin rolls input edges and resets the draw list and
// per-frame "hot". end resolves leftover state (clears hot if nothing claimed
// it, drops active on mouse-up).
void wg_begin(wg_context *ctx, float screen_w, float screen_h, float dt);
void wg_end(wg_context *ctx);

// id generation. seed from a string (usually the label) mixed with the current
// id-stack top so the same label in two panels is still unique.
wg_id wg_gen_id(wg_context *ctx, const char *label);
wg_id wg_gen_id_n(wg_context *ctx, const char *label, int salt);

void  wg_push_id(wg_context *ctx, wg_id id);
void  wg_pop_id(wg_context *ctx);

// the shared interaction kernel. given a widget's id and rect, update hot/active
// and report what happened. `out_hovered`/`out_held` may be NULL.
// returns 1 on a completed "click" (press inside, release inside).
int wg_behavior(wg_context *ctx, wg_id id, wg_rect r,
                int *out_hovered, int *out_held);

int wg_is_hot(const wg_context *ctx, wg_id id);
int wg_is_active(const wg_context *ctx, wg_id id);

#endif
