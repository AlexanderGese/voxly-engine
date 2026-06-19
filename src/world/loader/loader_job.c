#include "loader_job.h"

loader_job loader_job_make(uint64_t key, loader_job_kind kind,
                           int priority, uint32_t serial, uint64_t now_us) {
    loader_job j;
    j.key         = key;
    j.kind        = kind;
    j.priority    = priority;
    j.serial      = serial;
    j.enqueued_us = now_us;
    return j;
}

loader_stage loader_job_target_stage(loader_job_kind kind) {
    switch (kind) {
        case LOADER_JOB_ALLOC:  return LOADER_STAGE_ALLOCED;
        case LOADER_JOB_GEN:    return LOADER_STAGE_GENERATED;
        case LOADER_JOB_LIGHT:  return LOADER_STAGE_LIT;
        case LOADER_JOB_MESH:   return LOADER_STAGE_MESHED;
        case LOADER_JOB_UPLOAD: return LOADER_STAGE_RESIDENT;
    }
    // unreachable, but the compiler likes a return here.
    return LOADER_STAGE_EMPTY;
}

int loader_job_for_stage(loader_stage stage) {
    switch (stage) {
        case LOADER_STAGE_EMPTY:     return LOADER_JOB_ALLOC;
        case LOADER_STAGE_ALLOCED:   return LOADER_JOB_GEN;
        case LOADER_STAGE_GENERATED: return LOADER_JOB_LIGHT;
        case LOADER_STAGE_LIT:       return LOADER_JOB_MESH;
        case LOADER_STAGE_MESHED:    return LOADER_JOB_UPLOAD;
        case LOADER_STAGE_RESIDENT:  return -1;   // done, nothing to do
    }
    return -1;
}

int loader_job_is_main_thread(loader_job_kind kind) {
    // upload pokes opengl, everything else is pure cpu and safe to thread later.
    return kind == LOADER_JOB_UPLOAD;
}

const char *loader_job_name(loader_job_kind kind) {
    switch (kind) {
        case LOADER_JOB_ALLOC:  return "alloc";
        case LOADER_JOB_GEN:    return "gen";
        case LOADER_JOB_LIGHT:  return "light";
        case LOADER_JOB_MESH:   return "mesh";
        case LOADER_JOB_UPLOAD: return "upload";
    }
    return "?";
}
