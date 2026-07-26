#include "ecs_snapshot.h"
#include <stdlib.h>
#include <string.h>
#include "../../config.h"
#include "../../util/log.h"
on overrun they
// return 0 and leave a flag so the caller bails. no partial-load nonsense.
typedef struct {
    const uint8_t *p;
    size_t         len;
    size_t         off;
    int            bad;
} blob_reader;
