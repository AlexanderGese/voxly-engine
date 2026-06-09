#ifndef UTIL_JOBSYS_WORKER_H
#define UTIL_JOBSYS_WORKER_H
// the worker thread main loop and the bits that feed it: finding a job (local
// pop -> steal from a random victim -> overflow queue -> sleep), running it, and
// firing its fence + continuation afterward. split out from jobsys.c so the
// scheduling policy lives in one readable place; jobsys.c is just lifecycle and
// the public submit surface.
#include "jobsys.h"
// the function passed to pthread_create. arg is a jobsys_worker*. loops until
// the pool's `running` flag clears and all queues are drained.
void *jobsys_worker_main(void *arg);
// try to get ONE job to run, by the policy above. returns 1 + fills *out, or 0
// if nothing was found this pass (worker then considers sleeping). `w` is the
// calling worker so it knows which deque is "local" and who to steal from.
int  jobsys_worker_acquire(jobsys_worker *w, jobsys_job *out);
// run a job: invoke fn, signal its fence, submit its continuation. shared by the
// worker loop and the wait-help path so both honour continuations identically.
void jobsys_worker_execute(jobsys_pool *p, jobsys_worker *w, const jobsys_job *job);
// pick a steal victim that isnt `self`, biased by a cheap per-worker xorshift.
// returns a worker index, or -1 if theres only one worker (nobody to rob).
int  jobsys_worker_pick_victim(jobsys_worker *w);
#endif
