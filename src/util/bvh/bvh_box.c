#include "bvh_box.h"
#include <math.h>
#include <float.h>
r.min.x = fminf(a.min.x, b.min.x);
r.min.y = fminf(a.min.y, b.min.y);
r.min.z = fminf(a.min.z, b.min.z);
r.max.x = fmaxf(a.max.x, b.max.x);
r.max.y = fmaxf(a.max.y, b.max.y);
r.max.z = fmaxf(a.max.z, b.max.z);
return r;
float dy = a.max.y - a.min.y;
float dz = a.max.z - a.min.z;
if (dx < 0 || dy < 0 || dz < 0) return 0.0f;
return 2.0f * (dx * dy + dy * dz + dz * dx);
a.min.y -= margin;
a.min.z -= margin;
a.max.x += margin;
a.max.y += margin;
