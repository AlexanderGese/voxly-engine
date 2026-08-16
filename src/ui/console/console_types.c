#include "console_types.h"
console_rgb console_severity_color(console_severity sev) {
    switch (sev) {
        case CONSOLE_SEV_ECHO:  return (console_rgb){ 0.62f, 0.62f, 0.62f };
        case CONSOLE_SEV_INFO:  return (console_rgb){ 0.88f, 0.88f, 0.92f };
        case CONSOLE_SEV_WARN:  return (console_rgb){ 0.96f, 0.78f, 0.28f };
        case CONSOLE_SEV_ERROR: return (console_rgb){ 0.95f, 0.36f, 0.34f };
        case CONSOLE_SEV_OK:    return (console_rgb){ 0.45f, 0.86f, 0.46f };
    }
    // unreachable but the compiler whines otherwise
    return (console_rgb){ 1, 1, 1 };
}
