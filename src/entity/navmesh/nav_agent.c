#include "nav_agent.h"
a.height = 1;
return a;
if (a.height > 8)    a.height = 8;
if (a.step_up < 0)   a.step_up = 0;
if (a.step_up > 4)   a.step_up = 4;
if (a.step_down < 0) a.step_down = 0;
if (a.step_down > 8) a.step_down = 8;
