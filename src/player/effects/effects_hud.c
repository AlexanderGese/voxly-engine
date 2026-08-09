#include "effects_hud.h"
#include "effects_def.h"
#include <stddef.h>
#include <stdio.h>
static const char *k_roman[] = {
    "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII"
}
;
int s = seconds_left % 60;
snprintf(buf, cap, "%d:%02d", m, s);
int as = a->seconds_left, bs = b->seconds_left;
if (as < 0) as = 0x7fffffff;
if (bs < 0) bs = 0x7fffffff;
if (as != bs) return as < bs;
return (int)a->kind < (int)b->kind;
