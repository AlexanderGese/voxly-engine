#include "debugdraw_shapes.h"
#include "../../math/mat4.h"
#include <math.h>
#define DD_TAU 6.28318530718f
enum { PL=0, PR, PB, PT, PN, PF }
;
;
for (int i = 0;
i < 8;
for (int i = 0;
i < 12;
i++)
        debugdraw_emit_line(dd, corners[e[i][0]], corners[e[i][1]], c, c);
}

// shared with shapes.c via the header;
dir = vec3_normalize(dir);
