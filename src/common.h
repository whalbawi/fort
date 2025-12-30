#ifndef FORT_COMMON_H
#define FORT_COMMON_H

#include <errno.h>     // for errno
#include <limits.h>    // for PATH_MAX
#include <stdarg.h>    // for va_end, va_list, va_start
#include <stdio.h>     // for fprintf, size_t, stderr, snprintf, vfprintf, NULL
#include <stdlib.h>    // for getenv, mkstemp
#include <string.h>    // for strerrorname_np, memcpy
#include <unistd.h>    // for close
#include <sys/stat.h>  // for stat

#define FORT_UNUSED(x) (void)(x)

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

typedef enum {
    FORT_OUTCOME_OK,
    FORT_OUTCOME_ERR,
    FORT_OUTCOME_FATAL,
} fort_outcome_t;

#define FORT_OUTCOME_NOK_RET(expr)                                                                 \
    {                                                                                              \
        fort_outcome_t outcome__ = (expr);                                                         \
        if (outcome__ != FORT_OUTCOME_OK) {                                                        \
            return outcome__;                                                                      \
        }                                                                                          \
    }

typedef struct {
    const char* p;
    size_t len;
} buf_t;

typedef struct {
    char path[PATH_MAX];
    size_t len;
} filepath_t;

static inline void veprintln(const char* fmt, va_list args) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    FORT_UNUSED(vfprintf(stderr, fmt, args));
#pragma GCC diagnostic pop

    va_end(args);
    FORT_UNUSED(fprintf(stderr, "\n"));
}

static inline void eprint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    FORT_UNUSED(vfprintf(stderr, fmt, args));
#pragma GCC diagnostic pop
    va_end(args);
}

static inline void eprintln(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    veprintln(fmt, args);
    va_end(args);
}

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR,
    LOG_FATAL,

} fort_log_level_t;

static inline const char* ARGlog_level(fort_log_level_t level) {
    switch (level) {

    case LOG_DEBUG:
        return "debug";
    case LOG_INFO:
        return "info";
    case LOG_ERROR:
        return "error";
    case LOG_FATAL:
        return "fatal";
    default:
        return "unknown";
    }
}
static inline void fort_log(fort_log_level_t level, const char* fmt, ...) {
#ifndef NDEBUG
    if (level == LOG_DEBUG) {
        return;
    }
#endif
    FORT_UNUSED(fprintf(stderr, "%s: ", ARGlog_level(level)));
    va_list args;
    va_start(args, fmt);
    veprintln(fmt, args);
    va_end(args);
}

#define FMTerrno "errno=%s"

#define ARGerrno strerrorname_np(errno)

#define FORT_CLOSE(fd, filepath)                                                                   \
    {                                                                                              \
        int res = close(fd);                                                                       \
        if (res < 0) {                                                                             \
            fort_log(                                                                              \
                LOG_ERROR, "could not close file: %s. %s:%d", (filepath), __FILE__, __LINE__);     \
        }                                                                                          \
    }

static inline fort_outcome_t tmp_file(filepath_t* filepath) {
    const char* tmp_dir = getenv("TMPDIR");
    if (tmp_dir == NULL) {
        tmp_dir = "/tmp";
        struct stat st = {0};
        if (stat(tmp_dir, &st) == -1) {
            return FORT_OUTCOME_FATAL;
        }
    }

    char template[PATH_MAX];
    int needed = snprintf(template, NELEM(template), "%s/fort_XXXXXX", tmp_dir);
    if (needed < 0) {
        fort_log(LOG_FATAL, "could not produce temp file template: " FMTerrno, ARGerrno);
        return FORT_OUTCOME_FATAL;
    }

    size_t len = (size_t)needed;
    if (len > NELEM(template)) {
        fort_log(LOG_FATAL,
                 "insufficient buffer for temporary file name: %zu vs %zu",
                 len,
                 NELEM(template));
        return FORT_OUTCOME_FATAL;
    }

    int fd = mkstemp(template);
    if (fd == -1) {
        fort_log(LOG_FATAL, "could create temp file: " FMTerrno, ARGerrno);
        return FORT_OUTCOME_FATAL;
    }

    FORT_UNUSED(close(fd));
    FORT_UNUSED(memcpy(filepath->path, template, len));
    filepath->len = len;

    return FORT_OUTCOME_OK;
}

#endif // FORT_COMMON_H
