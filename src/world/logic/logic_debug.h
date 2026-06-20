#ifndef WORLD_LOGIC_DEBUG_H
#define WORLD_LOGIC_DEBUG_H

#include "logic_net.h"

// debug + introspection for the wire-logic net. none of this is on the hot
// path; it's for the F3-style overlay and for poking at the network from a
// console. no GL here - the actual overlay rendering lives in render/, this
// just hands it numbers and strings.

// counts of each cell kind, plus aggregates. zero-init and fill via
// logic_debug_stats.
typedef struct {
    int total;
    int wires;
    int sources;
    int gates;
    int repeaters;
    int lamps;
    int powered;       // cells with output > 0
    int max_power;     // strongest single output in the net
    int scheduled;     // events currently queued
    int throttled;     // cells being held back as runaway clocks
} logic_debug_stats;

void logic_debug_collect(const logic_net *net, logic_debug_stats *out);

// one-line summary of a single cell, e.g. "AND @(12,64,-3) f=+x p=15". writes
// up to cap bytes into buf (always null-terminated). returns bytes written.
int  logic_debug_cell_line(const logic_cell *c, char *buf, int cap);

// dump the whole net to the log at LOG_DEBUG, one line per cell. capped so a
// giant network doesn't spam forever; returns how many lines were printed.
int  logic_debug_dump(const logic_net *net, int max_lines);

// a single character glyph for a cell, for ascii overlays / map views.
char logic_debug_glyph(const logic_cell *c);

#endif
