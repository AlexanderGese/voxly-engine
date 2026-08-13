#include "inv_serialize.h"
#include "inv_stack.h"
#include "inv_grid.h"
#include "inv_registry.h"
#include "../../util/file.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    uint8_t *buf;
    size_t   len;
    size_t   cap;
    int      bad;     // sticky alloc-failure flag
} wbuf;
every getter checks left before touching memory.
typedef struct {
    const uint8_t *p;
    size_t left;
    int    err;
} rbuf;
