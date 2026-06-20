#include "logic_debug.h"
#include "logic_block.h"
#include "logic_gate.h"
#include "logic_clock.h"
#include "logic_dir.h"
#include "../../util/log.h"
#include <stddef.h>
#include <stdio.h>

void logic_debug_collect(const logic_net *net, logic_debug_stats *out) {
    out->total = 0;
    out->wires = 0;
    out->sources = 0;
    out->gates = 0;
    out->repeaters = 0;
    out->lamps = 0;
    out->powered = 0;
    out->max_power = 0;
    out->scheduled = logic_queue_len(&net->queue);
    out->throttled = logic_clock_throttled_count(&net->clock);

    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &net->grid.map);
    while (hm_iter_next(&it, &key, &val)) {
        logic_cell *c = (logic_cell *)val;
        out->total++;
        if (logic_block_is_wire(c->kind))     out->wires++;
        if (logic_block_is_source(c->kind))   out->sources++;
        if (logic_block_is_gate(c->kind))     out->gates++;
        if (c->kind == LOGIC_BLOCK_REPEATER)  out->repeaters++;
        if (c->kind == LOGIC_BLOCK_LAMP)      out->lamps++;
        if (c->power > 0)                     out->powered++;
        if (c->power > out->max_power)        out->max_power = c->power;
    }
}

int logic_debug_cell_line(const logic_cell *c, char *buf, int cap) {
    if (cap <= 0) return 0;

    const logic_block_info *bi = logic_block_get(c->kind);
    const char *name = bi->name;

    // gates report their boolean op rather than the generic "and/or/.." name.
    int gk = logic_block_gate_kind(c->kind);
    if (gk != LOGIC_GATE_COUNT) name = logic_gate_name((logic_gate_kind)gk);

    const char *face = logic_block_is_directional(c->kind)
                     ? logic_dir_name((logic_dir)c->facing) : "-";

    int n = snprintf(buf, (size_t)cap, "%s @(%d,%d,%d) f=%s p=%u%s",
                     name, c->x, c->y, c->z, face, c->power,
                     (c->flags & LOGIC_CF_STAGED) ? " *" : "");
    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;
    return n;
}

int logic_debug_dump(const logic_net *net, int max_lines) {
    char line[96];
    int printed = 0;

    logic_debug_stats st;
    logic_debug_collect(net, &st);
    LOGD("logic net: %d cells (%d wire, %d gate, %d rep, %d lamp), %d powered, "
         "tick=%u, queued=%d",
         st.total, st.wires, st.gates, st.repeaters, st.lamps,
         st.powered, net->tick, st.scheduled);

    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &net->grid.map);
    while (hm_iter_next(&it, &key, &val)) {
        if (printed >= max_lines) {
            LOGD("  ... (%d more cells)", st.total - printed);
            break;
        }
        logic_cell *c = (logic_cell *)val;
        logic_debug_cell_line(c, line, sizeof line);
        LOGD("  %s", line);
        printed++;
    }
    return printed;
}

char logic_debug_glyph(const logic_cell *c) {
    switch (c->kind) {
        case LOGIC_BLOCK_WIRE:     return c->power > 0 ? '+' : '.';
        case LOGIC_BLOCK_SOURCE:   return 'S';
        case LOGIC_BLOCK_LAMP:     return c->power > 0 ? 'O' : 'o';
        case LOGIC_BLOCK_REPEATER: return '>';
        case LOGIC_BLOCK_TORCH:    return c->power > 0 ? 'T' : 't';
        case LOGIC_BLOCK_GATE_AND: return '&';
        case LOGIC_BLOCK_GATE_OR:  return '|';
        case LOGIC_BLOCK_GATE_XOR: return '^';
        case LOGIC_BLOCK_GATE_NOT: return '!';
        case LOGIC_BLOCK_BUTTON:   return 'b';
        case LOGIC_BLOCK_LEVER:    return c->power > 0 ? 'L' : 'l';
        default:                   return '?';
    }
}
