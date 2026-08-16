#include "console_cvar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
if (console_cvar_find(t, name)) return -1;
console_cvar *v = &t->vars[t->count++];
v->name = name;
v->type = type;
v->ptr  = ptr;
v->min  = min;
v->max  = max;
v->help = help;
return 0;
break;
case CVAR_BOOL:
            snprintf(out, cap, "%s", (*(int*)v->ptr) ? "true" : "false");
break;
