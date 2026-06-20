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
schedule_neighbourhood(net, x, y, z, 1);
if (!c || c->kind != LOGIC_BLOCK_BUTTON) return;
c->power = logic_block_emit(c->kind);
c->phase = LOGIC_BUTTON_HOLD;
c->flags |= LOGIC_CF_LATCHED;
net->dirty_all = 1;
schedule_neighbourhood(net, x, y, z, 0);
schedule(net, x, y, z, LOGIC_BUTTON_HOLD);
int budget = LOGIC_TICK_BUDGET;
if (wmoved) active += wmoved;
net->dirty_all = 0;
hm_iter it;
uint64_t key;
void *val;
hm_iter_init(&it, &net->grid.map);
net->last_active = active;
return active;
}
