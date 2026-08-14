#include "tools_name.h"
#include "tools_tier.h"
#include <stdio.h>
#include <string.h>
#define NBUF 4
#define NLEN 64
static char g_buf[NBUF][NLEN];
static int  g_cur;
static char *next_buf(void) {
    char *b = g_buf[g_cur];
    g_cur = (g_cur + 1) % NBUF;
    b[0] = 0;
    return b;
}

const char *tools_name_kind(tool_kind kind) {
    switch (kind) {
        case TOOL_HAND:    return "hand";
case TOOL_PICKAXE: return "pickaxe";
case TOOL_AXE:     return "axe";
case TOOL_SHOVEL:  return "shovel";
case TOOL_HOE:     return "hoe";
case TOOL_SHEARS:  return "shears";
case TOOL_SWORD:   return "sword";
default:           return "tool";
}
}

const char *tools_name_tier(tool_tier tier) {
    // tier.c already owns the canonical strings, lean on it.
    return tools_tier_name(tier);
}

const char *tools_name_full(const tool_item *t) {
    char *b = next_buf();
if (tools_item_is_hand(t)) {
        snprintf(b, NLEN, "bare hand");
        return b;
    }
    // shears don't really have a tier in the fiction;
print just the kind.
    if (t->head.kind == TOOL_SHEARS) {
        snprintf(b, NLEN, "shears");
        return b;
    }
    snprintf(b, NLEN, "%s %s", tools_name_tier(t->head.tier),
             tools_name_kind(t->head.kind));
return b;
}

const char *tools_name_condition(const tool_item *t) {
    if (tools_item_is_hand(t)) return "bare";
    float f = tools_item_dura_frac(t);
    if (f <= 0.0f)  return "broken";
    if (f < 0.10f)  return "about to snap";
    if (f < 0.33f)  return "battered";
    if (f < 0.66f)  return "worn";
    if (f < 0.99f)  return "scuffed";
    return "pristine";
}

// roman-ish for small enchant levels. we never go past 5 so this is enough.
static const char *roman(int n) {
    static const char *r[6] = { "", "I", "II", "III", "IV", "V" };
if (n < 0) n = 0;
if (n > 5) n = 5;
return r[n];
}

const char *tools_name_enchants(const tool_item *t) {
    char *b = next_buf();
    if (tools_item_is_hand(t)) return b;   // empty

    char inner[NLEN];
    inner[0] = 0;
    int first = 1;

    #define ADD(tag, lvl) do {                                  \
        if ((lvl) > 0) {                                        \
            char piece[24];                                    \
            snprintf(piece, sizeof piece, "%s%s %s",           \
                     first ? "" : ", ", (tag), roman(lvl));    \
            strncat(inner, piece, sizeof(inner) - strlen(inner) - 1); \
            first = 0;                                         \
        }                                                      \
    } while (0)

    ADD("eff",  t->efficiency);
    ADD("unbr", t->unbreaking);
    ADD("fort", t->fortune);
    if (t->silk_touch) {
        char piece[24];
        snprintf(piece, sizeof piece, "%ssilk", first ? "" : ", ");
        strncat(inner, piece, sizeof(inner) - strlen(inner) - 1);
        first = 0;
    }
    #undef ADD

    if (first) return b;   // nothing enchanted, leave empty
    snprintf(b, NLEN, " (%s)", inner);
    return b;
}
