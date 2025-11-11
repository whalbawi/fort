#ifndef FORT_COMMON_H
#define FORT_COMMON_H

#define FORT_UNUSED(x) (void)(x)

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

void eprintln(const char* fmt, ...);

#endif // FORT_COMMON_H
