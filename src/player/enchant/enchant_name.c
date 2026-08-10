#include "enchant_name.h"
#include "enchant_registry.h"
#include "../../math/rng.h"

#include <string.h>

size_t enchant_name_roman(int n, char *buf, size_t cap) {
    if (!buf || cap == 0) return 0;
    // we only ever go up to ENCHANT_MAX_LEVEL (10), so a tiny table beats a
    // general roman-numeral algorithm.
    static const char *tab[11] = {
        "", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix", "x"
    };
    if (n < 0) n = 0;
    if (n > 10) {
        // past the table, just print the number. nobody enchants this high
        // but the byte ceiling allows it.
        int len = 0;
        char tmp[8];
        int v = n;
        do { tmp[len++] = (char)('0' + v % 10); v /= 10; } while (v && len < 7);
        size_t o = 0;
        while (len > 0 && o + 1 < cap) buf[o++] = tmp[--len];
        buf[o] = '\0';
        return o;
    }
    const char *s = tab[n];
    size_t o = 0;
    while (s[o] && o + 1 < cap) { buf[o] = s[o]; o++; }
    buf[o] = '\0';
    return o;
}

size_t enchant_name_label(enchant_id id, int level, char *buf, size_t cap) {
    if (!buf || cap == 0) return 0;
    buf[0] = '\0';
    const enchant_def *d = enchant_registry_get(id);
    if (!d || !d->name) return 0;

    size_t o = 0;
    const char *nm = d->name;
    while (nm[o] && o + 1 < cap) { buf[o] = nm[o]; o++; }
    buf[o] = '\0';

    // single-level enchants (silk, infinity, aqua) read better without "i".
    if (level > 1 && d->max_level > 1 && o + 2 < cap) {
        buf[o++] = ' ';
        char rom[8];
        size_t rl = enchant_name_roman(level, rom, sizeof rom);
        for (size_t i = 0; i < rl && o + 1 < cap; ++i) buf[o++] = rom[i];
        buf[o] = '\0';
    }
    return o;
}

size_t enchant_name_galactic(uint64_t seed, int glyphs, char *buf, size_t cap) {
    if (!buf || cap == 0) return 0;
    if (glyphs < 0) glyphs = 0;

    // the galactic font occupies the lowercase+space slots of the atlas font
    // row; we just emit pseudo-random lowercase letters and the renderer maps
    // them through that row. spaces sprinkled in so it reads like words.
    rng r;
    rng_init(&r, seed ? seed : 0x9E3779B97F4A7C15ull);

    size_t o = 0;
    int since_space = 0;
    for (int i = 0; i < glyphs && o + 1 < cap; ++i) {
        // bias toward a space roughly every 3-5 glyphs.
        if (since_space >= 3 && rng_range(&r, 0, 3) == 0) {
            buf[o++] = ' ';
            since_space = 0;
            continue;
        }
        buf[o++] = (char)('a' + rng_range(&r, 0, 25));
        since_space++;
    }
    buf[o] = '\0';
    return o;
}
