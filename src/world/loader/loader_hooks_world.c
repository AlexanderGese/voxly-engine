#include "loader_hooks_world.h"
#include "../chunk.h"
#include "../worldgen.h"
#include "../lighting.h"
#include "../../render/mesher.h"
#include "../../config.h"
if (!c) return LOADER_FAIL;
if (c->generated) return LOADER_OK;
