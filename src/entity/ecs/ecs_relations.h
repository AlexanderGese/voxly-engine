#ifndef ENTITY_ECS_RELATIONS_H
#define ENTITY_ECS_RELATIONS_H
#include "ecs_world.h"
#include "../../util/hashmap.h"
// entity-to-entity links. the component model is great for "this entity has
// these traits" but bad at "this entity belongs to that one" -- a minecart and
// its rider, a turret and the boss it's bolted to, a mob and the spawner that
// owns it. baking a parent field into a component meant every system that
// touched it had to special-case the despawn-the-kids-too logic, so it lives
// here as its own little graph instead.
//
// storage is a hashmap from entity-index -> a small node holding parent + a
// linked list of children (via first_child / next_sibling indices). entities
// never have that many kids so the intrusive sibling list is plenty; no darray
// churn per parent.
typedef struct {
    ecs_entity self;
    ecs_entity parent;
    ecs_entity first_child;
    ecs_entity next_sibling;
    uint32_t   child_count;
} ecs_rel_node;
typedef struct {
    hashmap    nodes;     // entity-index -> ecs_rel_node*
    uint32_t   links;     // total parented entities, for stats
} ecs_relations;
void ecs_relations_init(ecs_relations *r);
void ecs_relations_free(ecs_relations *r);
// make `child` a child of `parent`. re-parents if it already had one. passing
// ECS_NULL parent just detaches.
void ecs_set_parent(ecs_relations *r, ecs_entity child, ecs_entity parent);
void ecs_unparent(ecs_relations *r, ecs_entity child);
ecs_entity ecs_parent_of(const ecs_relations *r, ecs_entity child);
uint32_t   ecs_child_count(const ecs_relations *r, ecs_entity parent);
// iterate children. start with ECS_NULL, feed the previous return back in; gets
// ECS_NULL when done.  for (c = ecs_first_child(r,p); c; c = ecs_next_child(r,c))
ecs_entity ecs_first_child(const ecs_relations *r, ecs_entity parent);
ecs_entity ecs_next_child(const ecs_relations *r, ecs_entity child);
// destroy `e` and everything beneath it in the hierarchy, depth-first. routes
// every destroy through ecs_destroy so the world's deferral still applies.
void ecs_destroy_tree(ecs_world *w, ecs_relations *r, ecs_entity e);
// drop any links that point at entities the world has since reaped. the graph
// doesnt get told when ecs_destroy runs, so call this once a frame (or after a
// snapshot load) to garbage-collect dangling nodes.
void ecs_relations_prune(ecs_relations *r, const ecs_world *w);
#endif
