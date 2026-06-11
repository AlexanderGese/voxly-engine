#include "serialize.h"

const char *serialize_strerror(serialize_err e) {
    switch (e) {
        case SERIALIZE_OK:              return "ok";
        case SERIALIZE_ERR_OOM:         return "out of memory";
        case SERIALIZE_ERR_UNDERRUN:    return "buffer underrun";
        case SERIALIZE_ERR_BADTAG:      return "bad tag";
        case SERIALIZE_ERR_BADVERSION:  return "unsupported version";
        case SERIALIZE_ERR_CORRUPT:     return "corrupt payload";
        case SERIALIZE_ERR_OVERFLOW:    return "value out of range";
    }
    return "???";
}

int serialize_host_is_le(void) {
    // classic trick. const so the compiler folds it but we keep it runtime
    // because it costs nothing and i dont fully trust the macros.
    static const uint16_t probe = 0x0001;
    return *(const uint8_t *)&probe == 0x01;
}
