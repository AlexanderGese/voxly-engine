#ifndef WORLD_SKY_SEASON_H
#define WORLD_SKY_SEASON_H
typedef enum {
    VOXL_SEASON_SPRING = 0,
    VOXL_SEASON_SUMMER,
    VOXL_SEASON_AUTUMN,
    VOXL_SEASON_WINTER,
    VOXL_SEASON_COUNT
} voxl_sky_season_id;
typedef struct {
    voxl_sky_season_id id;
    float progress;     // 0..1 through the current season
    float temperature;  // -1 (cold) .. 1 (hot)
    float snow_bias;    // 0..1 chance precip falls as snow
    float day_length;   // hours of daylight (rough)
} voxl_sky_season;
voxl_sky_season voxl_sky_season_at(long day, int days_per_season);
const char *voxl_sky_season_name(voxl_sky_season_id s);
#endif
