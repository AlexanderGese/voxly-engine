#ifndef RENDER_GBUFFER_LIGHT_H
#define RENDER_GBUFFER_LIGHT_H

#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../config.h"

// the light list the accumulation pass consumes. mostly point lights coming
// from emissive blocks (torches, lava) plus one directional sun. we cull and
// sort these on the cpu before handing a trimmed set to the gpu.

// hard cap on lights uploaded per frame. anything past this gets dropped by
// the cull (it sorts by influence first so we drop the dim/far ones).
#define GBUFFER_MAX_LIGHTS 128

typedef enum {
    GBUF_LIGHT_POINT = 0,
    GBUF_LIGHT_SUN   = 1,   // directional, position is treated as -direction
} gbuffer_light_kind;

typedef struct {
    gbuffer_light_kind kind;
    vec3  pos;        // world pos (point) or direction toward source (sun)
    vec3  color;      // linear rgb, already premultiplied by intensity
    float radius;     // influence radius in blocks (point only)
    float intensity;  // scalar, kept separate so we can re-sort cheaply
} gbuffer_light;

typedef struct {
    gbuffer_light *items;   // darray
    int            sun_set; // whether a sun light is present
    gbuffer_light  sun;
} gbuffer_light_list;

void gbuffer_light_list_init(gbuffer_light_list *l);
void gbuffer_light_list_free(gbuffer_light_list *l);
void gbuffer_light_list_clear(gbuffer_light_list *l);

// add a point light. radius is derived from intensity if radius<=0.
void gbuffer_light_add_point(gbuffer_light_list *l, vec3 pos, vec3 color,
                             float intensity, float radius);

// set the single directional sun light.
void gbuffer_light_set_sun(gbuffer_light_list *l, vec3 dir, vec3 color);

// distance at which a point light's attenuation falls below `cutoff`.
// used to build the light's bounding box for frustum/scissor culling.
float gbuffer_light_range(const gbuffer_light *li, float cutoff);

// world-space aabb of a point light's influence sphere.
aabb gbuffer_light_bounds(const gbuffer_light *li);

#endif
