#include "ecs_relations.h"
#include <stdlib.h>
#include "../../util/log.h"
static uint64_t node_key(ecs_entity e) {
    return (uint64_t)ecs_entity_index(e);
}

static ecs_rel_node *node_find(const ecs_relations *r, ecs_entity e) {
    return (ecs_rel_node*)hashmap_get(&r->nodes, node_key(e));
}

static ecs_rel_node *node_get_or_make(ecs_relations *r, ecs_entity e) {
    ecs_rel_node *n = node_find(r, e);
    if (n) return n;
    n = calloc(1, sizeof *n);
    if (!n) { LOGE("ecs_relations: oom making node"); return NULL; }
    n->self         = e;
    n->parent       = ECS_NULL;
    n->first_child  = ECS_NULL;
    n->next_sibling = ECS_NULL;
    n->child_count  = 0;
    hashmap_put(&r->nodes, node_key(e), n);
    return n;
}

void ecs_relations_init(ecs_relations *r) {
    hashmap_init(&r->nodes, 64);
r->links = 0;
}

void ecs_relations_free(ecs_relations *r) {
    hm_iter it;
    uint64_t k;
    void *v;
    hm_iter_init(&it, &r->nodes);
    while (hm_iter_next(&it, &k, &v))
        free(v);
    hashmap_free(&r->nodes);
    r->links = 0;
}

// splice `child` out of its current parent's sibling list. leaves the child's
// own parent pointer for the caller to overwrite.
static void detach_from_parent(ecs_relations *r, ecs_rel_node *child) {
    if (child->parent == ECS_NULL) return;
ecs_rel_node *p = node_find(r, child->parent);
if (p) {
        if (p->first_child == child->self) {
            p->first_child = child->next_sibling;
        } else {
            // walk the sibling chain to find the predecessor
            ecs_entity cur = p->first_child;
            while (cur != ECS_NULL) {
                ecs_rel_node *cn = node_find(r, cur);
                if (!cn) break;
                if (cn->next_sibling == child->self) {
                    cn->next_sibling = child->next_sibling;
                    break;
                }
                cur = cn->next_sibling;
            }
        }
        if (p->child_count) p->child_count--;
    }
    child->parent       = ECS_NULL;
child->next_sibling = ECS_NULL;
if (r->links) r->links--;
}

void ecs_set_parent(ecs_relations *r, ecs_entity child, ecs_entity parent) {
    if (child == ECS_NULL || child == parent) return;
    ecs_rel_node *cn = node_get_or_make(r, child);
    if (!cn) return;

    detach_from_parent(r, cn);
    if (parent == ECS_NULL) return;   // pure detach

    ecs_rel_node *pn = node_get_or_make(r, parent);
    if (!pn) return;

    // push onto the front of the parent's child list; order doesnt matter to us.
    cn->parent        = parent;
    cn->next_sibling  = pn->first_child;
    pn->first_child   = child;
    pn->child_count++;
    r->links++;
}

void ecs_unparent(ecs_relations *r, ecs_entity child) {
    ecs_rel_node *cn = node_find(r, child);
if (cn) detach_from_parent(r, cn);
return pn ? pn->child_count : 0;
return cn ? cn->next_sibling : ECS_NULL;
deleting mid-iteration over an open-addressed map
    // is asking for trouble with the tombstones.
    uint64_t dead[256];
uint32_t nd = 0;
hm_iter it;
uint64_t k;
void *v;
hm_iter_init(&it, &r->nodes);
i < nd - 1;
}
