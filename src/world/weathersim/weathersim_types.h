#ifndef WORLD_WEATHERSIM_TYPES_H
#define WORLD_WEATHERSIM_TYPES_H

#include <stdint.h>
#include "../../config.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"

// shared plain-data for the weather simulator. the whole thing is a coarse
// 2d grid laid over the loaded world: every cell carries pressure, two
// temperatures (air + a slower-moving "ground" memory), humidity and a wind
// vector. fronts are advected blobs that push that field around; precip is
// what falls out of the field where humidity beats the dew point.
//
// nothing in here touches a chunk or the renderer. the public ctx samples the
// field for the existing world/weather.h enum so the rest of the engine never
// has to know this lives underneath it. i kept it 2d on purpose — real weather
// has vertical structure but voxl's sky is a flat lid, so a stack of planes is
// plenty and an order of magnitude cheaper than a real fluid solve.

// the sim grid is coarse: one cell spans this many chunks on a side. fronts
// are big and slow, so resolving them per-chunk would be a waste. 4 chunks
// (64 blocks) per cell felt like the right grain.
#define WEATHERSIM_CHUNKS_PER_CELL  4
#define WEATHERSIM_BLOCKS_PER_CELL  (WEATHERSIM_CHUNKS_PER_CELL * CHUNK_SIZE_X)

// grid dimensions. the field covers a square region centered on the player and
// is recentered as they wander. odd so there's a true center cell.
#define WEATHERSIM_DIM   33
#define WEATHERSIM_CELLS (WEATHERSIM_DIM * WEATHERSIM_DIM)

// reference pressure in millibars-ish units. the sim works in deviations from
// this; a front is just a travelling low or high.
#define WEATHERSIM_P_REF      1013.0f
// physical-ish constants, fudged until the sky looked right. not NWS-grade.
#define WEATHERSIM_DEW_SLOPE  4.6f    // how fast saturation rises with temp
#define WEATHERSIM_LAPSE      0.0065f // temp drop per meter of altitude

// the eight weather fronts we can have in flight at once. more than this and
// they just smear into noise, fewer and the sky gets boring.
#define WEATHERSIM_MAX_FRONTS 8

// what a front *is* climatically. warm fronts gently raise temp + humidity and
// bring drizzle; cold fronts slam through with a pressure drop and heavier
// showers; occluded ones are the tired remnants that just spin down.
typedef enum {
    WEATHERSIM_FRONT_WARM = 0,
    WEATHERSIM_FRONT_COLD,
    WEATHERSIM_FRONT_OCCLUDED,
    WEATHERSIM_FRONT_STATIONARY,
    WEATHERSIM_FRONT_COUNT
} weathersim_front_kind;

// lifecycle of a single front. the front state machine walks these in order;
// a front never goes backwards, it only decays.
typedef enum {
    WEATHERSIM_LIFE_FORMING = 0,  // pressure anomaly deepening, no precip yet
    WEATHERSIM_LIFE_MATURE,       // full strength, dumping precip
    WEATHERSIM_LIFE_DECAYING,     // filling back in, precip tapering
    WEATHERSIM_LIFE_DEAD          // slot free for reuse
} weathersim_life;

// a single travelling pressure system. position is in *cell* space (float so it
// glides between cells), velocity in cells/second. radius shapes the gaussian
// footprint it stamps onto the pressure + humidity planes.
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

// one coarse grid cell. plain numbers, swept linearly every tick.
typedef struct {
    float pressure;   // deviation from WEATHERSIM_P_REF, mb
    float temp;       // air temperature, celsius-ish
    float ground_t;   // slow ground temperature, lags air (thermal inertia)
    float humidity;   // 0..1 relative
    vec2  wind;       // blocks/sec horizontal wind at this cell
    float accum;      // precipitation accumulated this cell since last drain
    float cloud;      // 0..1 cloud cover, tracks humidity with hysteresis
} weathersim_cell;

// the precip kinds we resolve to. maps onto the engine's weather_state but is
// finer-grained internally (we distinguish drizzle from a downpour for the
// particle budget even though world/weather only knows rain vs snow).
typedef enum {
    WEATHERSIM_PRECIP_NONE = 0,
    WEATHERSIM_PRECIP_DRIZZLE,
    WEATHERSIM_PRECIP_RAIN,
    WEATHERSIM_PRECIP_DOWNPOUR,
    WEATHERSIM_PRECIP_SLEET,
    WEATHERSIM_PRECIP_SNOW
} weathersim_precip;

// the knobs. seeded once per world. defaults in weathersim_default_params().
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

// what the public query hands back for a world position. this is the bridge to
// the rest of the engine — render/particles read this, not the raw field.
typedef struct {
    weathersim_precip precip;
    float  intensity;     // 0..1, drives particle density + fog
    float  temperature;   // celsius-ish at the queried altitude
    float  humidity;      // 0..1
    float  cloud;         // 0..1 cover, for sky tint
    float  pressure;      // absolute mb, for a barometer hud if anyone wants it
    vec2   wind;          // blocks/sec, drives particle drift + ambient sfx
    float  accum;         // standing accumulation at the column
} weathersim_sample;

// sane starting knobs derived off the world seed. tweak the returned struct
// afterward if you want angrier or calmer skies.
weathersim_params weathersim_default_params(uint32_t seed);

#endif
