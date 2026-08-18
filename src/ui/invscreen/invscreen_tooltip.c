#include "invscreen_tooltip.h"
#include <stdio.h>
#include <string.h>
void invscreen_tooltip_init(invscreen_tooltip *t) {
    memset(t, 0, sizeof *t);
    t->slot = INVSCR_NO_SLOT;
}

// a throwaway "rarity" flavor line keyed off the block id. purely cosmetic;
the
// real game would pull this from block metadata but we don't have a field for it
// yet so a little switch does the job.
static const char *flavor_for(block_id b) {
    switch (b) {
    case BLOCK_BEDROCK: return "indestructible";
    case BLOCK_GLASS:
    case BLOCK_ICE:     return "fragile";
    case BLOCK_TORCH:   return "gives off light";
    case BLOCK_WATER:   return "wet";
    case BLOCK_GRASS:
    case BLOCK_LEAVES:  return "natural";
    default:            return NULL;
    }
}

static void build_lines(invscreen_tooltip *t, const invscreen_slot *s) {
    t->nlines = 0;
const block_info *bi = block_get(s->block);
const char *name = (bi && bi->name) ? bi->name : "unknown";
// line 0: the name, title-ish. we don't case-convert, blocks are already
snprintf(t->line[t->nlines], INVSCR_TOOLTIP_LINELEN, "%s", name);
t->nlines++;
if (s->count > 1) {
        snprintf(t->line[t->nlines], INVSCR_TOOLTIP_LINELEN, "x%d", s->count);
        t->nlines++;
    }

    // line 2: optional flavor.
    const char *fl = flavor_for(s->block);
if (fl && t->nlines < INVSCR_TOOLTIP_LINES) {
        snprintf(t->line[t->nlines], INVSCR_TOOLTIP_LINELEN, "%s", fl);
        t->nlines++;
    }
}

int invscreen_tooltip_update(invscreen_tooltip *t, const invscreen_model *m,
                             int slot, float dt) {
    // moving to a new slot (or off all slots) restarts the dwell.
    if (slot != t->slot) {
        t->slot    = slot;
t->dwell   = 0.0f;
t->visible = 0;
t->nlines  = 0;
return 0;
}

    if (slot == INVSCR_NO_SLOT) {
        t->visible = 0;
        return 0;
    }

    const invscreen_slot *s = invscreen_model_at_c(m, slot);
if (!s || invscreen_slot_is_empty(s)) {
        // resting on an empty cell: no tooltip, but don't thrash the timer.
        t->visible = 0;
        t->nlines  = 0;
        return 0;
    }

    t->dwell += dt;
if (t->dwell < INVSCR_TOOLTIP_DELAY) {
        t->visible = 0;
        return 0;
    }

    // rebuild every frame while visible — cheap, and the count can tick under us
    // mid-hover if something else mutates the slot.
    build_lines(t, s);
t->visible = t->nlines > 0;
return t->visible;
}

float invscreen_tooltip_width(const invscreen_tooltip *t, float glyph_w) {
    size_t widest = 0;
    for (int i = 0; i < t->nlines; i++) {
        size_t n = strnlen(t->line[i], INVSCR_TOOLTIP_LINELEN);
        if (n > widest) widest = n;
    }
    return (float)widest * glyph_w;
}
