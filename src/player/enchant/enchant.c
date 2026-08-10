#include "enchant.h"
#include "../../util/log.h"

// the subsystem's single mutable bit of global state: whether init ran. the
// registry owns its own readiness flag; this just gates the rest of the api
// and gives the boot sequence one obvious call.

static int g_ready = 0;

int enchant_init(void) {
    if (g_ready) return enchant_registry_count();
    int n = enchant_registry_init();
    if (n <= 0) {
        LOGE("enchant: registry came up empty, enchanting disabled");
        return 0;
    }
    g_ready = 1;
    LOGI("enchant: %d kinds registered", n);
    return n;
}

int enchant_ready(void) {
    return g_ready;
}
