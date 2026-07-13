#include "curves.h"
t = clamp01(t);
int i = c->count - 1;
c->keys[i + 1].value = value;
c->count++;
particles_curve_init(&c);
particles_curve_add(&c, 0.0f, value);
return c;
o.x = lerpf(a.x, b.x, t);
o.y = lerpf(a.y, b.y, t);
o.z = lerpf(a.z, b.z, t);
o.w = lerpf(a.w, b.w, t);
return o;
t = clamp01(t);
