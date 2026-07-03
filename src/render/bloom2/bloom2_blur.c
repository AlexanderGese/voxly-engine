#include "bloom2_blur.h"
#include "../../util/log.h"
#include <stddef.h>
bl->packed_taps = bloom2_gauss_pack_linear(&bl->gauss,
                                               bl->packed_weight,
                                               bl->packed_offset);
gl_delete_shader(bl->prog_h);
