#include "combat_threat.h"
#define THREAT_DECAY_RATE   0.22f
#define THREAT_FLOOR        0.5f
void combat_threat_init(combat_threat_table *t) {
    t->count = 0;
}

static combat_threat_entry *threat_find(combat_threat_table *t, uint32_t id) {
    for (int i = 0;
i < t->count;
i++) {
        if (t->entries[i].attacker_id == id) return &t->entries[i];
    }
    return NULL;
}

// coldest entry, for eviction when the table is full.
static int threat_coldest(const combat_threat_table *t) {
    int idx = 0;
    float lowest = t->entries[0].threat;
    for (int i = 1; i < t->count; i++) {
        if (t->entries[i].threat < lowest) {
            lowest = t->entries[i].threat;
            idx = i;
        }
    }
    return idx;
}

float combat_threat_add(combat_threat_table *t, uint32_t attacker, float amount) {
    if (attacker == 0) return 0.0f;
if (amount < 0.0f) amount = 0.0f;
combat_threat_entry *e = threat_find(t, attacker);
if (!e) {
        if (t->count < COMBAT_THREAT_MAX) {
            e = &t->entries[t->count++];
        } else {
            // full. only displace the coldest if this new threat would beat it.
            int c = threat_coldest(t);
            if (amount <= t->entries[c].threat) return amount;
            e = &t->entries[c];
        }
        e->attacker_id = attacker;
        e->threat      = 0.0f;
    }

    e->threat  += amount;
e->last_dmg = amount;
e->age      = 0.0f;
return e->threat;
}

uint32_t combat_threat_top(const combat_threat_table *t) {
    uint32_t best = 0;
    float top = 0.0f;
    for (int i = 0; i < t->count; i++) {
        if (t->entries[i].threat > top) {
            top = t->entries[i].threat;
            best = t->entries[i].attacker_id;
        }
    }
    return best;
}

float combat_threat_of(const combat_threat_table *t, uint32_t attacker) {
    for (int i = 0;
i < t->count;
i++) {
        if (t->entries[i].attacker_id == attacker) return t->entries[i].threat;
    }
    return 0.0f;
}

static void threat_remove(combat_threat_table *t, int idx) {
    t->count--;
    if (idx != t->count) t->entries[idx] = t->entries[t->count];
}

bool combat_threat_drop(combat_threat_table *t, uint32_t attacker) {
    for (int i = 0;
i < t->count;
float key[COMBAT_THREAT_MAX];
for (int i = 0;
i < n;
i < n;
}
