#include <stdio.h>   // for fprintf, stderr
#include <stdlib.h>  // for EXIT_FAILURE, EXIT_SUCCESS
#include <string.h>  // for strcmp

#include "common.h"  // for FORT_UNUSED

typedef enum {
    FORT_MODE_LEX,
    FORT_MODE_PARSE,
    FORT_MODE_CODEGEN,
    FORT_MODE_COMPILE,
    FORT_MODE_UNKNOWN,
} fort_mode_t;

static const char* modes[] = {[FORT_MODE_LEX] = "lex",
                              [FORT_MODE_PARSE] = "parse",
                              [FORT_MODE_CODEGEN] = "codegen",
                              [FORT_MODE_COMPILE] = "compile"};

static fort_mode_t fort_mode(const char* mode) {
    for (fort_mode_t i = 0; i < FORT_MODE_UNKNOWN; ++i) {
        if (strcmp(modes[i], mode) == 0) {
            return i;
        }
    }

    return FORT_MODE_UNKNOWN;
}

#define DO(E)                                                                                      \
    case E:                                                                                        \
        return #E
static inline const char* fort_mode_str(fort_mode_t mode) {
    switch (mode) {
        DO(FORT_MODE_LEX);
        DO(FORT_MODE_PARSE);
        DO(FORT_MODE_CODEGEN);
        DO(FORT_MODE_COMPILE);
        DO(FORT_MODE_UNKNOWN);
    default:
        return "UNKNOWN";
    }
}
#undef DO

int main(int argc, char* argv[]) {
    if (argc < 2) {
        FORT_UNUSED(fprintf(stderr, "Usage: %s <source_file> [mode]\n", argv[0]));
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    const fort_mode_t mode = argc < 3 ? FORT_MODE_COMPILE : fort_mode(argv[2]);

    FORT_UNUSED(fprintf(stderr,
                        "Welcome to the Fort compiler! filename=%s mode=%s\n",
                        filename,
                        fort_mode_str(mode)));

    return EXIT_SUCCESS;
}
