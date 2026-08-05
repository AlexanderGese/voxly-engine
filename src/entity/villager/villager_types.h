#ifndef ENTITY_VILLAGER_TYPES_H
#define ENTITY_VILLAGER_TYPES_H
#include <stdint.h>
// shared vocabulary for the villager subsystem. no logic here, just the
// nouns. everyone downstream speaks in these enums so we don't pass loose
// ints around and then wonder six files later what "3" meant.
// professions. UNEMPLOYED is the default a baby grows into; NITWIT never
// gets a job and is happy about it. the rest each bind to one workstation
// block and unlock a trade table.
typedef enum {
    VILLAGER_PROF_UNEMPLOYED = 0,
    VILLAGER_PROF_FARMER,        // composter
    VILLAGER_PROF_LIBRARIAN,     // lectern
    VILLAGER_PROF_BLACKSMITH,    // forge / anvil-ish
    VILLAGER_PROF_BUTCHER,       // smoker
    VILLAGER_PROF_MASON,         // cutting table
    VILLAGER_PROF_CLERIC,        // brewing-ish altar
    VILLAGER_PROF_NITWIT,        // no job, no shame
    VILLAGER_PROF_COUNT
} villager_profession;
// what the villager is *trying* to do this moment. the schedule maps the
// day clock onto one of these; the brain turns it into movement + actions.
typedef enum {
    VILLAGER_ACT_SLEEP = 0,
    VILLAGER_ACT_WORK,
    VILLAGER_ACT_GATHER,    // meet at the village center / bell
    VILLAGER_ACT_WANDER,    // idle pottering about
    VILLAGER_ACT_PANIC,     // something scary, run home
    VILLAGER_ACT_COUNT
} villager_activity;
// a point of interest the villager cares about. beds to sleep in, work
// sites to claim, and the bell that defines the village center.
typedef enum {
    VILLAGER_POI_BED = 0,
    VILLAGER_POI_WORKSTATION,
    VILLAGER_POI_BELL,
    VILLAGER_POI_COUNT
} villager_poi_kind;
// gossip flavours. these accumulate per-player reputation and nudge prices.
// minor positive comes from trading; major positive from curing; the
// negatives come from hitting villagers or killing them nearby.
typedef enum {
    VILLAGER_GOSSIP_MINOR_POSITIVE = 0,
    VILLAGER_GOSSIP_MAJOR_POSITIVE,
    VILLAGER_GOSSIP_TRADING,
    VILLAGER_GOSSIP_MINOR_NEGATIVE,
    VILLAGER_GOSSIP_MAJOR_NEGATIVE,
    VILLAGER_GOSSIP_COUNT
} villager_gossip_kind;
// caps so reputation can't run away to infinity. tuned by feel.
#define VILLAGER_GOSSIP_MAX        100
#define VILLAGER_GOSSIP_DECAY_DAY   2   // points shed per in-game day
#define VILLAGER_WALLET_MAX        64
#define VILLAGER_DAY_SLOTS         24
#endif
