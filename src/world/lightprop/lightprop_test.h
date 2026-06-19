#ifndef WORLD_LIGHTPROP_TEST_H
#define WORLD_LIGHTPROP_TEST_H

// in-tree smoke tests for the light propagator. not a real framework, just a
// function that bangs on the pure bits (queue, codec, step math) and returns the
// number of failures. wired into the dev build's `--selftest` path; ships as a
// no-op everywhere else. keeps me honest after i "just tweak the falloff".

// returns 0 if everything passed, else the count of failed checks. logs each
// failure with file/line via LOGE.
int lightprop_selftest(void);

#endif
