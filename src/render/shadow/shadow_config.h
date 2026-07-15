#ifndef RENDER_SHADOW_CONFIG_H
#define RENDER_SHADOW_CONFIG_H
// cascaded shadow map tunables. all the knobs live here so i dont have to
// go hunting through six files when the shadows look like garbage again.
// number of cascades. 4 is the sweet spot for our render distance. going
// to 5 barely helped and cost another depth pass.
#define SHADOW_CASCADE_COUNT     4
// per-cascade depth map resolution (square). same for every cascade — i
// tried shrinking the far ones and the popping wasnt worth the bytes saved.
#define SHADOW_MAP_SIZE          2048
#define SHADOW_MAX_DISTANCE      180.0f
#define SHADOW_NEAR_OFFSET       0.5f
#define SHADOW_SPLIT_LAMBDA      0.75f
#define SHADOW_BOUNDS_PAD        2.5f
#define SHADOW_LIGHT_PULLBACK    64.0f
#define SHADOW_DEPTH_BIAS_CONST  1.25f
#define SHADOW_DEPTH_BIAS_SLOPE  2.75f
#define SHADOW_NORMAL_BIAS       0.04f
#define SHADOW_PCF_RADIUS        2
#define SHADOW_PCF_JITTER        1
#define SHADOW_FADE_BAND         0.12f
#endif
