#include "navmesh.h"
#include "nav_query.h"
#include "../../config.h"
#include "../../util/log.h"
#include <math.h>
#include <string.h>
int cz = floordiv(wz, CHUNK_SIZE_Z);
*pcx = floordiv(cx, NAV_PATCH_CHUNKS) * NAV_PATCH_CHUNKS;
*pcz = floordiv(cz, NAV_PATCH_CHUNKS) * NAV_PATCH_CHUNKS;
i < NAV_CACHE_PATCHES;
i++)
        nav_grid_free(&nm->patches[i].grid);
