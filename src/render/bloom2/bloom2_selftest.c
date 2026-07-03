#include "bloom2_selftest.h"
#include "bloom2_gauss.h"
#include "bloom2_chain.h"
#include "bloom2_tint.h"
#include "bloom2_params.h"
#include "../../util/log.h"
#include <math.h>
bloom2_gauss_build(&g, 0.0f);
float sum = 0.0f;
for (int i = 0;
i < g.taps;
i++) sum += g.weight[i];
i < g.taps / 2;
i < n;
return 0;
}
    return 1;
bloom2_params_default(&p);
p.knee = 0.0f;
