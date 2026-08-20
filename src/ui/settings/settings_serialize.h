#ifndef UI_SETTINGS_SERIALIZE_H
#define UI_SETTINGS_SERIALIZE_H

// read/write the model's live values as a key=value text blob, one per line.
// matches the format game/settings.h uses so a host can hand the same file to
// either. we serialize `live` (the committed values) not `work` — you don't want
// half-finished edits hitting disk.
//
// these work on in-memory buffers; the host owns actual file io (util/file.h).

#include "settings_model.h"

#include <stddef.h>

// write live values into `out` (capacity `cap`, always null-terminated). returns
// the number of bytes that *would* have been written excluding the terminator,
// like snprintf — so >= cap means it was truncated.
size_t settings_serialize_write(const settings_model *m, char *out, size_t cap);

// parse a key=value blob into the model's work copy (then the caller can apply).
// unknown keys are skipped, malformed lines are skipped, missing keys keep their
// current value. returns the count of recognized keys applied.
int settings_serialize_read(settings_model *m, const char *text);

// parse a single "key=value" line into work. returns 1 if a known key was set.
// exposed mainly so the loader can stream line by line if it wants.
int settings_serialize_line(settings_model *m, const char *line);

#endif
