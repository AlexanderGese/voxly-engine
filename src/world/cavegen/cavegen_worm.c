#include "cavegen_worm.h"
#include "cavegen_noise.h"
#include "../../config.h"
#include "../../math/vec3.h"
#include <math.h>
int cx = (int)floorf(c.x), cy = (int)floorf(c.y), cz = (int)floorf(c.z);
float r2 = r * r;
for (int dy = -ir;
dy <= ir;
w->steps_left--;
int wx, wy, wz;
cavegen_cell_to_world(g, (int)w->pos.x, (int)w->pos.y, (int)w->pos.z,
                          &wx, &wy, &wz);
float fx = (float)wx, fy = (float)wy, fz = (float)wz;
float s = 0.08f;
float gx = cavegen_value3(fx*s + 11.0f, fy*s, fz*s, p->seed ^ 0x770001u);
float gy = cavegen_value3(fx*s, fy*s + 19.0f, fz*s, p->seed ^ 0x770002u);
float gz = cavegen_value3(fx*s, fy*s, fz*s + 7.0f, p->seed ^ 0x770003u);
vec3 turn = vec3_new(gx, gy * 0.5f, gz);
w->dir = vec3_add(w->dir, vec3_scale(turn, p->worm_pitch_max));
if (w->dir.y >  0.7f) w->dir.y =  0.7f;
if (w->dir.y < -0.8f) w->dir.y = -0.8f;
w->dir = vec3_normalize(w->dir);
