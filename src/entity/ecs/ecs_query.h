#ifndef ENTITY_ECS_QUERY_H
#define ENTITY_ECS_QUERY_H

#include "ecs_world.h"

// query: iterate every live entity that has (at least) a set of components, and
// optionally lacks others. we drive iteration off the *smallest* matching store
// so a "has rare-component AND common-component" query walks the rare one --
// classic sparse-set trick, keeps it roughly O(min store) instead of O(all
// entities).

#define ECS_QUERY_MAX_GET  8

typedef struct {
    ecs_world    *w;
    ecs_signature require;     // must have all of these
    ecs_signature exclude;     // must have none of these

    const ecs_store *lead;     // store we iterate over
    uint32_t      cursor;      // dense index into lead

    ecs_entity    e;           // current entity (valid after _next == 1)

    // cached fetch list so the hot loop doesnt re-walk the signature each step
    ecs_cmp       fetch[ECS_QUERY_MAX_GET];
    int           fetch_n;
} ecs_query;

// begin a query. `require`/`exclude` are signatures built with ecs_sig_set.
void ecs_query_begin(ecs_query *q, ecs_world *w,
                     ecs_signature require, ecs_signature exclude);

// advance to the next match. returns 0 when exhausted.
int  ecs_query_next(ecs_query *q);

// fetch a component pointer for the current entity. cheap -- after a match we
// already know the entity has it, but it still does the sparse lookup.
void *ecs_query_get(ecs_query *q, ecs_cmp c);

// convenience: count matches without a manual loop. consumes nothing, walks a
// throwaway cursor.
uint32_t ecs_query_count(ecs_world *w, ecs_signature require,
                         ecs_signature exclude);

// tiny helpers so callers can build signatures inline:
// ecs_with(ECS_CMP_TRANSFORM, ECS_CMP_VELOCITY, -1)
ecs_signature ecs_with(int first, ...);

#endif
