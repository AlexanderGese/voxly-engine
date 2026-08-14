#ifndef PLAYER_TOOLS_REGISTRY_H
#define PLAYER_TOOLS_REGISTRY_H
#include "tools_types.h"
#include "../../world/block.h"
// maps concrete block ids onto material classes so the mining math has
// something to chew on. populated once at startup. anything not registered
// falls back to a sane default based on the legacy block_ext "tool_required".
#define TOOLS_MAX_BLOCK_IDS 256
void       tools_registry_init(void);
mat_class  tools_registry_class(block_id id);
float      tools_registry_hardness(block_id id);
int        tools_registry_level(block_id id);
void       tools_registry_set(block_id id, mat_class cls, float hardness, int level);
#endif
