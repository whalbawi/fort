#include "common.h"

#include <stdarg.h>  // for va_end, va_list, va_start
#include <stdio.h>   // for fprintf, stderr, vfprintf

void eprintln(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    FORT_UNUSED(vfprintf(stderr, fmt, args));
    va_end(args);
    FORT_UNUSED(fprintf(stderr, "\n"));
}
