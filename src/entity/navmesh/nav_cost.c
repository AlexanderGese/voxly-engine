#include "nav_cost.h"
#include "nav_link.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NAV_STEP_BASE  10
cf->area = NULL;
cf->count = 0;
float r2 = radius * radius;
for (int i = 0;
i < g->count && i < cf->count;
if (li < 0 || li >= c->link_count) return NAV_COST_BLOCKED;
if (f && !nav_filter_allows(f, g, ci, li)) return NAV_COST_BLOCKED;
int to = c->link_to[li];
int kind = c->link_kind[li];
int base = NAV_STEP_BASE + c->link_cost[li] + nav_link_cost(kind);
int area = NAV_COST_NEUTRAL;
if (cf && cf->area && to < cf->count) area = cf->area[to];
if (area < 1) area = 1;
long scaled = (long)base * (long)area;
if (scaled >= NAV_COST_BLOCKED) scaled = NAV_COST_BLOCKED - 1;
