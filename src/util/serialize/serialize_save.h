#ifndef UTIL_SERIALIZE_SAVE_H
#define UTIL_SERIALIZE_SAVE_H

// the convenience layer the rest of the engine actually calls. it wires the
// doc header + tag sections + file io together so callers dont touch the
// writer/reader directly for the common case.
//
// a save file is: doc header, then a 'META' section, then any number of
// chunk/item/container sections, then the END sentinel.

#include "serialize_writer.h"
#include "serialize_reader.h"
#include "../../world/chunk.h"

#define SERIALIZE_SAVE_USER_VERSION 1

// metadata that goes in the 'META' section. world seed, spawn, play time.
typedef struct {
    uint64_t seed;
    float    spawn_x, spawn_y, spawn_z;
    double   play_seconds;
    uint32_t chunk_count;     // hint, may be 0 if unknown at write time
} serialize_save_meta;

// --- streaming save builder ----------------------------------------------
// open a builder, push sections, then commit to disk.
typedef struct {
    serialize_writer w;
    int              committed;
} serialize_save;

void serialize_save_open(serialize_save *s, const serialize_save_meta *meta);
void serialize_save_add_chunk(serialize_save *s, const chunk *c, int with_light);
void serialize_save_add_raw(serialize_save *s, serialize_writer *section_src);
int  serialize_save_commit(serialize_save *s, const char *path);
void serialize_save_abort(serialize_save *s);

// --- loading -------------------------------------------------------------
// loads a whole file into memory and validates the doc header + META. on
// success *meta is filled and *out_data / *out_len hold the malloc'd file
// buffer (caller frees) with *body_at pointing past the header so you can
// drive a serialize_reader over the remaining sections yourself.
int serialize_save_load(const char *path, serialize_save_meta *meta,
                        uint8_t **out_data, size_t *out_len, size_t *body_at);

// helper: given a loaded buffer, iterate chunk sections, calling cb per chunk.
// returns the number of chunks delivered, or -1 on a corrupt stream.
int serialize_save_each_chunk(const uint8_t *data, size_t len, size_t body_at,
                              void (*cb)(const chunk *c, void *ud), void *ud);

#endif
