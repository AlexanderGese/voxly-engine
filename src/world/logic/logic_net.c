#include "logic_net.h"
#include "logic_block.h"
#include "logic_signal.h"
#include "logic_gate.h"
#include "logic_repeater.h"
#include "logic_propagate.h"
#include "logic_dir.h"
#include <stddef.h>
#define LOGIC_BUTTON_HOLD 4
void logic_net_init(logic_net *net) {
    logic_grid_init(&net->grid);
    logic_queue_init(&net->queue);
    logic_clock_init(&net->clock);
    net->tick = 0;
    net->dirty_all = 0;
    net->last_active = 0;
}

void logic_net_free(logic_net *net) {
    logic_grid_free(&net->grid);
logic_queue_init(&net->queue);
}

int logic_net_at_rest(const logic_net *net) {
    return logic_queue_empty(&net->queue) && !net->dirty_all;
}

// schedule a single coord onto the queue.
static void schedule(logic_net *net, int x, int y, int z, uint32_t delay) {
    logic_queue_push(&net->queue, logic_key(x, y, z), net->tick, delay);
}

// schedule a coord and all six neighbours. the workhorse for "something here
// changed, let the area react".
static void schedule_neighbourhood(logic_net *net, int x, int y, int z,
                                   uint32_t delay) {
    schedule(net, x, y, z, delay);
    for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
        int nx, ny, nz;
        logic_dir_step((logic_dir)d, x, y, z, &nx, &ny, &nz);
        schedule(net, nx, ny, nz, delay);
    }
}

void logic_net_wake(logic_net *net, int x, int y, int z, uint32_t delay) {
    schedule_neighbourhood(net, x, y, z, delay);
}

logic_cell *logic_net_place(logic_net *net, int x, int y, int z,
                            block_id kind, logic_dir facing) {
    logic_cell *c = logic_grid_put(&net->grid, x, y, z, kind);
    if (!c) return NULL;
    c->kind = kind;
    if (logic_block_is_directional(kind))
        c->facing = (uint8_t)facing;
    // a freshly placed source starts emitting; the flood handles the rest.
    if (logic_block_is_source(kind) && !(c->flags & LOGIC_CF_LATCHED))
        c->power = 0; // lever/button start off; torch is handled on tick
    net->dirty_all = 1;
    schedule_neighbourhood(net, x, y, z, 0);
    return c;
}

void logic_net_remove(logic_net *net, int x, int y, int z) {
    if (logic_grid_remove(&net->grid, x, y, z)) {
        net->dirty_all = 1;
schedule_neighbourhood(net, x, y, z, 1);
}
}

void logic_net_set_source(logic_net *net, int x, int y, int z, int on) {
    logic_cell *c = logic_grid_get(&net->grid, x, y, z);
    if (!c || !logic_block_is_source(c->kind)) return;
    uint8_t want = on ? logic_block_emit(c->kind) : 0;
    if (c->power == want && (!!on == !!(c->flags & LOGIC_CF_LATCHED))) return;
    c->power = want;
    if (on) c->flags |= LOGIC_CF_LATCHED;
    else    c->flags &= ~LOGIC_CF_LATCHED;
    net->dirty_all = 1;
    schedule_neighbourhood(net, x, y, z, 0);
}

void logic_net_press(logic_net *net, int x, int y, int z) {
    logic_cell *c = logic_grid_get(&net->grid, x, y, z);
if (!c || c->kind != LOGIC_BLOCK_BUTTON) return;
c->power = logic_block_emit(c->kind);
c->phase = LOGIC_BUTTON_HOLD;
c->flags |= LOGIC_CF_LATCHED;
net->dirty_all = 1;
schedule_neighbourhood(net, x, y, z, 0);
schedule(net, x, y, z, LOGIC_BUTTON_HOLD);
}

// re-evaluate one non-wire cell. returns 1 if its committed output changed (or,
// for a repeater, if it armed/advanced a transition that we must follow up on).
static int eval_cell(logic_net *net, logic_cell *c) {
    logic_grid *g = &net->grid;

    if (logic_block_is_wire(c->kind) || logic_block_is_sink(c->kind))
        return 0; // wires settle via the flood; sinks have no output

    if (c->kind == LOGIC_BLOCK_BUTTON) {
        // tick the hold timer; pop back off when it runs out.
        if (c->flags & LOGIC_CF_LATCHED) {
            if (c->phase > 0) c->phase--;
            if (c->phase == 0) {
                c->flags &= ~LOGIC_CF_LATCHED;
                if (c->power != 0) { c->power = 0; return 1; }
            }
        }
        return 0;
    }

    if (c->kind == LOGIC_BLOCK_LEVER || c->kind == LOGIC_BLOCK_SOURCE) {
        // pure latching source: output tracks the latch flag, nothing to flood.
        uint8_t want = (c->flags & LOGIC_CF_LATCHED) ? logic_block_emit(c->kind) : 0;
        if (c->power != want) { c->power = want; return 1; }
        return 0;
    }

    if (c->kind == LOGIC_BLOCK_TORCH) {
        // inverter source: lit unless the block it leans on is powered.
        logic_dir back = logic_dir_opposite((logic_dir)c->facing);
        int bx, by, bz;
        logic_dir_step(back, c->x, c->y, c->z, &bx, &by, &bz);
        logic_cell *support = logic_grid_get(g, bx, by, bz);
        uint8_t in = support ? support->power : 0;
        uint8_t want = logic_invert(in);
        if (c->power != want) { c->power = want; return 1; }
        return 0;
    }

    if (c->kind == LOGIC_BLOCK_REPEATER) {
        int armed = logic_repeater_poke(g, c);
        int moved = logic_repeater_tick(g, c);
        // if still mid-countdown, keep ticking it.
        if (c->flags & LOGIC_CF_STAGED)
            schedule(net, c->x, c->y, c->z, 1);
        return moved || armed;
    }

    if (logic_block_is_gate(c->kind)) {
        uint8_t want = logic_gate_compute(g, c);
        if (c->power != want) { c->power = want; return 1; }
        return 0;
    }

    return 0;
}

int logic_net_tick(logic_net *net) {
    int active = 0;
int budget = LOGIC_TICK_BUDGET;
while (!logic_queue_empty(&net->queue) &&
           logic_queue_peek_tick(&net->queue) <= net->tick &&
           budget-- > 0) {
        logic_event ev;
        if (!logic_queue_pop(&net->queue, &ev)) break;

        // resolve the cell from its key. unpack isn't needed: get-by-key.
        logic_cell *c = (logic_cell *)hashmap_get(&net->grid.map, ev.key);
        if (!c) continue; // cell was removed since scheduling; ignore.

        if (eval_cell(net, c)) {
            active++;
            net->dirty_all = 1;
            // feed the flip into the clock watcher; a runaway hands back a
            // stretched delay so it can't melt the frame.
            uint32_t delay = logic_clock_observe(&net->clock, ev.key,
                                                 c->power > 0, net->tick);
            // wake the neighbourhood for next tick so the change propagates.
            schedule_neighbourhood(net, c->x, c->y, c->z, delay);
        }
    }

    // 2. settle the wire flood whenever anything emitter-side moved.
    if (net->dirty_all) {
        int wmoved = logic_propagate_wires(&net->grid);
if (wmoved) active += wmoved;
net->dirty_all = 0;
hm_iter it;
uint64_t key;
void *val;
hm_iter_init(&it, &net->grid.map);
net->last_active = active;
return active;
}
