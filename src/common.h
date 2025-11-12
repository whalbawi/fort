#ifndef FORT_COMMON_H
#define FORT_COMMON_H

#include <stdarg.h>  // for va_end, va_list, va_start
#include <stdio.h>   // for fprintf, stderr, vfprintf

#define FORT_UNUSED(x) (void)(x)

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

static inline void eprintln(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    FORT_UNUSED(vfprintf(stderr, fmt, args));
#pragma GCC diagnostic pop

    va_end(args);
    FORT_UNUSED(fprintf(stderr, "\n"));
}

#endif // FORT_COMMON_H
