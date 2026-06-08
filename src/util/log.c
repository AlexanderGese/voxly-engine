#define _POSIX_C_SOURCE 200809L
#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static log_level_t g_level = LOG_INFO;

static const char *level_tag(log_level_t l) {
    switch (l) {
    case LOG_DEBUG: return "DBG";
    case LOG_INFO:  return "INF";
    case LOG_WARN:  return "WRN";
    case LOG_ERROR: return "ERR";
    case LOG_FATAL: return "FTL";
    }
    return "???";
}

static const char *short_file(const char *path) {
    const char *s = path;
    for (const char *p = path; *p; p++) if (*p == '/') s = p + 1;
    return s;
}

void log_set_level(log_level_t lvl) {
    g_level = lvl;
}

void log_msg(log_level_t lvl, const char *file, int line, const char *fmt, ...) {
    if (lvl < g_level) return;

    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char ts[16];
    strftime(ts, sizeof ts, "%H:%M:%S", &tm);

    fprintf(stderr, "[%s %s %s:%d] ", ts, level_tag(lvl), short_file(file), line);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);

    if (lvl == LOG_FATAL) {
        fflush(stderr);
        exit(1);
    }
}
