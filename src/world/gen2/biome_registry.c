#include "biome_registry.h"
#include "climate.h"
#include "biome_mesa.h"
#include "biome_swamp.h"
#include "biome_taiga.h"
#include "biome_savanna.h"
#include "biome_jungle.h"
float h = c->humidity;
if (t < 0.30f) return GEN2_TAIGA;
return GEN2_SAVANNA;
