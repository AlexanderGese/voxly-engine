#ifndef WORLD_EROSION_NOISE_H
#define WORLD_EROSION_NOISE_H

#include <stdint.h>

// tiny self-contained hash + value noise for the erosion pass. same deal as
// gen2: kept separate from the engine perlin so reseeding rain spawns cant
// nudge global terrain. all pure, deterministic on (x,z,seed).

uint32_t erosion_hash2(int x, int z, uint32_t seed);

// hashed float in [0,1).
float erosion_hash_f01(int x, int z, uint32_t seed);

// per-droplet stream. we mix the droplet index in so two tiles with the same
// seed still spawn different rain. returns a fresh state word.
uint32_t erosion_stream_seed(uint32_t seed, int droplet_index);

// advance a private rng word and read a float in [0,1). pcg-ish, good enough.
float erosion_stream_f01(uint32_t *state);

// smooth value noise in [-1,1], used to give per-cell rock hardness some
// texture so erosion doesnt carve perfectly symmetric valleys.
float erosion_value2(float x, float z, uint32_t seed);

#endif
