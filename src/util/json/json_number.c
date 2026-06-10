#include "json_number.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the json number grammar, by hand:
// -? (0 | [1-9][0-9]*) ( . [0-9]+ )? ( [eE] [+-]? [0-9]+ )?
// we walk it once just to find the extent and learn whether it's an integer,
// then hand the slice to strtod for the actual value. doing the value parse
// ourselves would mean reimplementing rounding, no thanks.

size_t json_number_scan(const char *s, size_t n, double *out, int *is_int) {
    size_t i = 0;
    int isint = 1;

    if (i < n && s[i] == '-') i++;

    // integer part
    if (i >= n) return 0;
    if (s[i] == '0') {
        i++;  // a leading zero must stand alone (no 007)
    } else if (s[i] >= '1' && s[i] <= '9') {
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
    } else {
        return 0;  // no digits where the int part should be
    }

    // fraction
    if (i < n && s[i] == '.') {
        isint = 0;
        i++;
        if (i >= n || s[i] < '0' || s[i] > '9') return 0;  // need a digit
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
    }

    // exponent
    if (i < n && (s[i] == 'e' || s[i] == 'E')) {
        isint = 0;
        i++;
        if (i < n && (s[i] == '+' || s[i] == '-')) i++;
        if (i >= n || s[i] < '0' || s[i] > '9') return 0;  // need a digit
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
    }

    // hand the exact slice to strtod via a small stack copy so it cant run off
    // the end of a non-terminated buffer.
    char tmp[64];
    if (i >= sizeof tmp) {
        // absurdly long literal, fall back to a heap copy. rare.
        char *heap = (char *)malloc(i + 1);
        if (!heap) return 0;
        memcpy(heap, s, i);
        heap[i] = 0;
        *out = strtod(heap, NULL);
        free(heap);
    } else {
        memcpy(tmp, s, i);
        tmp[i] = 0;
        *out = strtod(tmp, NULL);
    }
    if (is_int) *is_int = isint;
    return i;
}

void json_number_format(strbuf *out, double v, int is_int) {
    // non-finite isnt representable in json. spec says no nan/inf, so we emit
    // null rather than write something a strict parser would choke on.
    if (!isfinite(v)) {
        strbuf_append(out, "null");
        return;
    }

    char buf[32];
    if (is_int && v >= -9.007199254740992e15 && v <= 9.007199254740992e15) {
        // inside the exact-integer range of a double; print as a plain integer.
        snprintf(buf, sizeof buf, "%lld", (long long)v);
        strbuf_append(out, buf);
        return;
    }

    snprintf(buf, sizeof buf, "%.17g", v);
    // %.17g is round-trip safe but ugly. try progressively shorter forms and
    // keep the shortest that still reads back to the same double.
    for (int prec = 1; prec < 17; prec++) {
        char shorter[32];
        snprintf(shorter, sizeof shorter, "%.*g", prec, v);
        if (strtod(shorter, NULL) == v) {
            strbuf_append(out, shorter);
            return;
        }
    }
    strbuf_append(out, buf);
}
