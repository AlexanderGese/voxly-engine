#ifndef WORLD_STRUCTGEN_RUIN_H
#define WORLD_STRUCTGEN_RUIN_H

#include "structgen_types.h"
#include "structgen_buffer.h"
#include "structgen_piece.h"

// ruin assembler. a single pillar-hall piece plus, sometimes, a smaller
// satellite hall offset to the side so a ruin reads as a compound rather than
// one lonely platform. trivial next to villages/dungeons but kept symmetrical
// so the driver can treat all three the same way.

void structgen_ruin_layout(structgen_plan *plan, const structgen_site *site);
int  structgen_ruin_generate(structgen_buffer *out, const structgen_site *site);

#endif
