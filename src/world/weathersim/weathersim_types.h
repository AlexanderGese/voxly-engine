#ifndef WORLD_WEATHERSIM_TYPES_H
#define WORLD_WEATHERSIM_TYPES_H
#include <stdint.h>
#include "../../config.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#define WEATHERSIM_CHUNKS_PER_CELL  4
#define WEATHERSIM_BLOCKS_PER_CELL  (WEATHERSIM_CHUNKS_PER_CELL * CHUNK_SIZE_X)
#define WEATHERSIM_DIM   33
#define WEATHERSIM_CELLS (WEATHERSIM_DIM * WEATHERSIM_DIM)
#define WEATHERSIM_P_REF      1013.0f
#define WEATHERSIM_DEW_SLOPE  4.6f    // how fast saturation rises with temp
#define WEATHERSIM_LAPSE      0.0065f // temp drop per meter of altitude
#define WEATHERSIM_MAX_FRONTS 8
typedef enum {
    WEATHERSIM_FRONT_WARM = 0,
    WEATHERSIM_FRONT_COLD,
    WEATHERSIM_FRONT_OCCLUDED,
    WEATHERSIM_FRONT_STATIONARY,
    WEATHERSIM_FRONT_COUNT
} weathersim_front_kind;
typedef enum {
    WEATHERSIM_LIFE_FORMING = 0,  // pressure anomaly deepening, no precip yet
    WEATHERSIM_LIFE_MATURE,       // full strength, dumping precip
    WEATHERSIM_LIFE_DECAYING,     // filling back in, precip tapering
    WEATHERSIM_LIFE_DEAD          // slot free for reuse
} weathersim_life;
typedef struct {
    weathersim_front_kind kind;
    weathersim_life       life;

    vec2  pos;        // cell-space center
    vec2  vel;        // cells/sec drift, set at spawn from the prevailing wind
    float radius;     // gaussian sigma in cells
    float depth;      // peak pressure anomaly (mb). signed: low<0, high>0

    float age;        // seconds since spawn
    float lifespan;   // seconds until it should be dead
    float strength;   // 0..1 envelope, ramps up then down over the lifespan

    uint32_t seed;    // per-front derived seed for jitter
} weathersim_front;
typedef struct {
    float pressure;   // deviation from WEATHERSIM_P_REF, mb
    float temp;       // air temperature, celsius-ish
    float ground_t;   // slow ground temperature, lags air (thermal inertia)
    float humidity;   // 0..1 relative
    vec2  wind;       // blocks/sec horizontal wind at this cell
    float accum;      // precipitation accumulated this cell since last drain
    float cloud;      // 0..1 cloud cover, tracks humidity with hysteresis
} weathersim_cell;
typedef enum {
    WEATHERSIM_PRECIP_NONE = 0,
    WEATHERSIM_PRECIP_DRIZZLE,
    WEATHERSIM_PRECIP_RAIN,
    WEATHERSIM_PRECIP_DOWNPOUR,
    WEATHERSIM_PRECIP_SLEET,
    WEATHERSIM_PRECIP_SNOW
} weathersim_precip;
typedef struct {
    uint32_t seed;

    // prevailing wind: the background drift the whole field is biased toward,
    // before fronts perturb it. blocks/sec.
    vec2  prevailing;

    // front spawning
    float front_rate;       // expected new fronts per minute
    float front_min_life;   // seconds, shortest lived front
    float front_max_life;   // seconds, longest lived front
    float front_min_radius; // cells
    float front_max_radius; // cells

    // field relaxation: how fast cells diffuse toward their neighbours and
    // relax back to the climate baseline. higher = mushier, calmer weather.
    float diffuse;          // 0..1 per tick blend
    float relax;            // 0..1 pull toward baseline

    // thermodynamics
    float ground_inertia;   // 0..1, how slowly ground_t chases air temp
    float humidity_gain;    // evaporation feed per second over water/ground
    float dew_bias;         // shifts the rain/snow threshold

    // precip
    float precip_threshold; // humidity over saturation needed to rain
    float accum_drain;      // accumulation bleeds off at this rate /sec
    float snow_temp;        // at/below this air temp precip falls as snow

    // sim cadence: we don't step the field every render frame, we accumulate
    // dt and step on a fixed tick so behaviour is framerate-independent.
    float tick_dt;          // seconds per sim tick
} weathersim_params;
#endif
