
#define _XOPEN_SOURCE 500 // NOLINT(bugprone-reserved-identifier,readability-identifier-naming)
#include <fcntl.h>     // for open, O_RDONLY
#include <getopt.h>    // for no_argument, getopt_long, option
#include <stdbool.h>   // for bool, false, true
#include <stdio.h>     // for perror, size_t
#include <stdlib.h>    // for EXIT_FAILURE, free, malloc, EXIT_SUCCESS
#include <unistd.h>    // for NULL, close, optind, pread, off_t, ssize_t
#include <sys/stat.h>  // for stat, fstat

#include "common.h"    // for eprintln, FORT_UNUSED
#include "lex.h"       // for tok_stream_fini, lexer_fini, lexer_run, mklexer

typedef enum {
    STAGE_LEX,
    STAGE_PARSE,
    STAGE_CODEGEN,
    STAGE_COMPILE,
} stage_t;

#define FMTstage "STAGE(%s)"

static inline const char* ARGstage(stage_t stage) {
    switch (stage) {
    case STAGE_LEX:
        return "lex";
    case STAGE_PARSE:
        return "parse";
    case STAGE_CODEGEN:
        return "codegen";
    case STAGE_COMPILE:
        return "compile";
    default:
        return "unknown";
    }
}

static char* load_src(const char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        FORT_UNUSED(close(fd));
        return NULL;
    }

    if (st.st_size < 0) {
        eprintln("error: unexpected file size: %zd", st.st_size);
        FORT_UNUSED(close(fd));
        return NULL;
    }

    size_t file_sz = (size_t)st.st_size;
    char* src = malloc(file_sz + 1);
    if (src == NULL) {
        perror("malloc");
        FORT_UNUSED(close(fd));
        return NULL;
    }
    src[file_sz] = '\0';

    off_t off = 0;
    size_t nbytes_rem = file_sz;
    while (nbytes_rem > 0) {
        ssize_t nbytes = pread(fd, src + off, nbytes_rem, off);
        if (nbytes < 0) {
            perror("pread");
            free(src);
            FORT_UNUSED(close(fd));
            return NULL;
        }
        nbytes_rem -= (size_t)nbytes;
        off += nbytes;
    }

    FORT_UNUSED(close(fd));
    return src;
}

static bool lex(const char* filepath, tok_stream_t* toks) {
    char* src = load_src(filepath);

    if (src == NULL) {
        return false;
    }

    lexer_t* lexer = mklexer(src, 0);
    *toks = lexer_run(lexer);
    lexer_fini(lexer);
    free(src);

    return true;
}

static void print_usage(void) {
    eprintln("Usage: fort [OPTIONS] <source_file>");
    eprintln("Options:");
    eprintln("  --lex       Tokenize the source file");
    eprintln("  --parse     Parse the source file\n");
    eprintln("  --codegen   Generate code from the source file");
    eprintln("  --compile   Compile the source file (default)");
}

typedef struct {
    const char* filepath;
    stage_t stage;
} opts_t;

static bool parse_opts(int argc, char* argv[], opts_t* opts) {
    static const struct option long_opts[] = {{"lex", no_argument, NULL, STAGE_LEX},
                                              {"parse", no_argument, NULL, STAGE_PARSE},
                                              {"codegen", no_argument, NULL, STAGE_CODEGEN},
                                              {"compile", no_argument, NULL, STAGE_COMPILE},
                                              {NULL, 0, NULL, 0}};
    int opt = -1;
    while ((opt = getopt_long(argc, argv, "", long_opts, NULL)) != -1) {
        switch (opt) {
        case STAGE_LEX:
        case STAGE_PARSE:
        case STAGE_CODEGEN:
        case STAGE_COMPILE:
            opts->stage = (stage_t)opt;
            break;
        default:
            return false;
        }
    }

    if (optind != argc - 1) {
        return false;
    }

    opts->filepath = argv[optind];

    return true;
}

int main(int argc, char* argv[]) {
    opts_t opts;
    bool valid = parse_opts(argc, argv, &opts);

    if (!valid) {
        print_usage();
        return EXIT_FAILURE;
    }

    switch (opts.stage) {
    case STAGE_LEX: {
        tok_stream_t toks;
        if (!lex(opts.filepath, &toks)) {
            eprintln("error: failed to read source file: %s", opts.filepath);
            return EXIT_FAILURE;
        }

        if (toks.err) {
            eprintln("error: failed to lex source file");
            tok_stream_fini(&toks);
            return EXIT_FAILURE;
        }

        tok_stream_fini(&toks);
        return EXIT_SUCCESS;
    }

    case STAGE_PARSE:
    case STAGE_CODEGEN:
    case STAGE_COMPILE:
        eprintln("not implemented: " FMTstage, ARGstage(opts.stage));
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}
