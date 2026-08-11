#ifndef PLAYER_FISHING_SERIAL_H
#define PLAYER_FISHING_SERIAL_H

#include <stddef.h>
#include <stdint.h>
#include "fishing_rod.h"
#include "fishing_stats.h"

// save/load for the bits of fishing state that outlive a session: the rod's
// stats and the career tally. little-endian byte stream with a magic+version
// header so an old save reads back cleanly (or is rejected). deliberately not
// hooked into the big serialize_doc layer — this is one flat blob the player
// save can embed wherever it likes.

#define FISHING_SERIAL_MAGIC    0x46495348u  /* 'FISH' */
#define FISHING_SERIAL_VERSION  1

// a write cursor over a caller-owned buffer. tracks how far we've written and
// flags overflow rather than scribbling past the end.
typedef struct {
    uint8_t *buf;
    size_t   cap;
    size_t   len;
    int      overflow;
} fishing_writer;

// a read cursor. flags underflow on a short/corrupt buffer.
typedef struct {
    const uint8_t *buf;
    size_t         cap;
    size_t         pos;
    int            underflow;
} fishing_reader;

void fishing_writer_init(fishing_writer *w, uint8_t *buf, size_t cap);
void fishing_reader_init(fishing_reader *r, const uint8_t *buf, size_t cap);

// write the rod+stats blob. returns bytes written, or 0 on overflow.
size_t fishing_serial_save(fishing_writer *w, const fishing_rod *rod,
                           const fishing_stats *stats);

// read it back. returns 0 on success, non-zero on bad magic/version/underflow.
int    fishing_serial_load(fishing_reader *r, fishing_rod *rod,
                           fishing_stats *stats);

#endif
