#ifndef RENDER_BLOOM2_SELFTEST_H
#define RENDER_BLOOM2_SELFTEST_H

// cpu-side sanity checks for the bloom2 math. no gl context required, so the
// engine can run these at startup (behind a debug flag) or from a test
// harness. each returns 1 on pass, 0 on fail and logs the first failure.
//
// these caught a couple of real bugs while i was writing the module — the
// linear-packing energy drift in particular was off by the center tap until
// i fixed the loop bounds.

// gaussian weights normalize to 1.
int bloom2_selftest_gauss_normalized(void);

// linear-packed taps preserve total energy (sum of packed == sum of full).
int bloom2_selftest_gauss_packed_energy(void);

// chain fit never returns more mips than fit, and clamps to >= 1.
int bloom2_selftest_chain_fit(void);

// saturate at sat=1 is identity; at sat=0 it's pure luma grey.
int bloom2_selftest_tint_saturate(void);

// knee curve degenerates correctly when knee == 0 (hard threshold).
int bloom2_selftest_knee_curve(void);

// runs everything. returns the count of failures (0 == all good).
int bloom2_selftest_run_all(void);

#endif
