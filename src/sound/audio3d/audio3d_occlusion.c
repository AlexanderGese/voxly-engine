#include "audio3d_occlusion.h"

#include <math.h>

void audio3d_occluder_init(audio3d_occluder *o, audio3d_solid_fn fn, void *user) {
    if (!o) return;
    o->solid     = fn;
    o->user      = user;
    o->max_steps = 64;          // ~ max_dist worth of blocks, plenty
    o->per_block = 0.34f;       // ~3 solid blocks and you're basically muffled
}

float audio3d_occlusion_estimate(const audio3d_occluder *o,
                                 vec3 listener, vec3 source) {
    if (!o || !o->solid) return 0.0f;   // no world hookup -> treat as open air

    vec3 d = vec3_sub(source, listener);
    float len = vec3_length(d);
    if (len < AUDIO3D_EPS) return 0.0f; // source on top of us, never occluded

    // amanatides-woo style voxel dda. step block to block along the ray and
    // ask the world if each cell is solid. we skip the source's own cell and
    // the listener's own cell so a source embedded in a wall isnt self-occluded.
    vec3 dir = vec3_scale(d, 1.0f / len);

    int x = (int)floorf(listener.x);
    int y = (int)floorf(listener.y);
    int z = (int)floorf(listener.z);
    int ex = (int)floorf(source.x);
    int ey = (int)floorf(source.y);
    int ez = (int)floorf(source.z);

    int stepx = dir.x > 0 ? 1 : (dir.x < 0 ? -1 : 0);
    int stepy = dir.y > 0 ? 1 : (dir.y < 0 ? -1 : 0);
    int stepz = dir.z > 0 ? 1 : (dir.z < 0 ? -1 : 0);

    // distance along the ray to the next cell boundary on each axis.
    float tmaxx = stepx ? ((float)(stepx > 0 ? x + 1 : x) - listener.x) / dir.x : 1e30f;
    float tmaxy = stepy ? ((float)(stepy > 0 ? y + 1 : y) - listener.y) / dir.y : 1e30f;
    float tmaxz = stepz ? ((float)(stepz > 0 ? z + 1 : z) - listener.z) / dir.z : 1e30f;

    float tdx = stepx ? (float)stepx / dir.x : 1e30f;
    float tdy = stepy ? (float)stepy / dir.y : 1e30f;
    float tdz = stepz ? (float)stepz / dir.z : 1e30f;

    int solids = 0;
    for (int i = 0; i < o->max_steps; i++) {
        if (x == ex && y == ey && z == ez) break;   // reached the source cell

        // advance to the nearest boundary.
        if (tmaxx <= tmaxy && tmaxx <= tmaxz) { x += stepx; tmaxx += tdx; }
        else if (tmaxy <= tmaxz)              { y += stepy; tmaxy += tdy; }
        else                                  { z += stepz; tmaxz += tdz; }

        if (x == ex && y == ey && z == ez) break;   // dont count the source cell
        if (o->solid(x, y, z, o->user)) solids++;
    }

    float occ = (float)solids * o->per_block;
    if (occ > 1.0f) occ = 1.0f;
    return occ;
}
