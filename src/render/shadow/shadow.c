#include "shadow.h"
#include "shadow_split.h"
#include "shadow_matrix.h"
#include "shadow_glsl.h"
#include "../../util/log.h"
shadow_pass_shutdown(&sh->pass);
shadow_map_destroy(&sh->map);
sh->ready = 0;
shadow_pass_render(&sh->pass, &sh->map, &sh->csm, draw, ctx);
