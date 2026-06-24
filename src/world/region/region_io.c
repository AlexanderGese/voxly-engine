#include "region_io.h"
#include "../../util/log.h"
#include <string.h>
if (len && fwrite(data, 1, len, f) != len) return -1;
uint32_t sectors = region_io_sectors_for(len);
size_t   padded  = (size_t)sectors * REGION_SECTOR_BYTES;
;
size_t pad = padded - len;
return 0;
