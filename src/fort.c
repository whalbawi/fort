#include <fcntl.h>     // for open, O_RDONLY
#include <getopt.h>    // for no_argument, getopt_long, option
#include <stdint.h>    // for uint64_t
#include <stdio.h>     // for perror, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS, free, NULL, exit
#include <string.h>    // for memcpy, strlen
#include <unistd.h>    // for NULL, close, execvp, fork, optind, pread, off_t
#include <sys/stat.h>  // for stat, fstat
#include <sys/wait.h>  // for waitpid

#include "assemble.h"  // for asm_prog_t, asm_prog_fini, assembler_fini, ass...
#include "common.h"    // for eprintln, FORT_OUTCOME_OK, fort_outcome_t, FOR...
#include "lex.h"       // for tok_stream_fini, tok_stream_t, lexer_fini, lex...
#include "parse.h"     // for prog_t, mkparser, parser_fini, parser_run, par...

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

static fort_outcome_t parse_opts(int argc, char* argv[], opts_t* opts) {
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
            return FORT_OUTCOME_ERR;
        }
    }

    if (optind != argc - 1) {
        return FORT_OUTCOME_ERR;
    }

    opts->filepath = argv[optind];

    return FORT_OUTCOME_OK;
}

static fort_outcome_t stage_lex(const char* src, tok_stream_t* toks) {
    lexer_t* lexer = mklexer(src, 0);
    fort_outcome_t outcome = lexer_run(lexer, toks);
    lexer_fini(lexer);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to lex source file");

        return outcome;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t stage_parse(const char* src, prog_t* prog) {
    tok_stream_t toks = {0};
    fort_outcome_t outcome = stage_lex(src, &toks);
    if (outcome != FORT_OUTCOME_OK) {
        return outcome;
    }

    parser_t* parser = mkparser(&toks);
    outcome = parser_run(parser, prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to parse source file");

        return outcome;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t stage_codegen(const char* src, asm_prog_t* asm_prog) {
    prog_t prog = {0};
    fort_outcome_t outcome = stage_parse(src, &prog);
    if (outcome != FORT_OUTCOME_OK) {
        return outcome;
    }

    assembler_t* assembler = mkassembler(&prog);
    outcome = assembler_run(assembler, asm_prog);
    assembler_fini(assembler);

    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to generate assembly");

        return outcome;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t assemble_and_link(const filepath_t* asm_file, const filepath_t* out_file) {
    pid_t pid = fork();
    if (pid == -1) {
        eprintln("could not fork process");
        return FORT_OUTCOME_FATAL;
    }

    if (pid == 0) {
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#else
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif
        char* const args[] = {
            "clang",
            "-x",
            "assembler",
            asm_file->path,
            "-o",
            out_file->path,
            NULL,
        };
#pragma GCC diagnostic pop

        if (execvp("clang", args) == -1) {
            eprintln("error: could not execute clang");
            exit(EXIT_FAILURE);
        }
    } else {
        int status = -1;
        FORT_UNUSED(waitpid(pid, &status, 0));
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            return FORT_OUTCOME_FATAL;
        }
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t stage_compile(const char* src, const filepath_t* out_file) {
    asm_prog_t prog = {0};
    fort_outcome_t outcome = stage_codegen(src, &prog);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to generate assembly");

        return outcome;
    }

    filepath_t asm_file = {0};
    outcome = tmp_file(&asm_file);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: could not create temporary assembly file");
    }

    outcome = emit_asm(&prog, &asm_file);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to emit assembly");

        return outcome;
    }

    outcome = assemble_and_link(&asm_file, out_file);
    if (outcome != FORT_OUTCOME_OK) {
        eprintln("error: failed to compile program");

        return outcome;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t compute_out_file(const char* src_file, filepath_t* out_file) {
    const uint64_t len = strlen(src_file);

    for (uint64_t i = 0; i < len; ++i) {
        if (src_file[i] == '.') {
            FORT_UNUSED(memcpy(out_file->path, src_file, i));
            out_file->len = i;
            return FORT_OUTCOME_OK;
        }
    }

    return FORT_OUTCOME_ERR;
}

int main(int argc, char* argv[]) {
    int exit_code = EXIT_FAILURE;
    fort_outcome_t outcome = FORT_OUTCOME_ERR;

    opts_t opts = {NULL, STAGE_COMPILE};
    outcome = parse_opts(argc, argv, &opts);
    if (outcome != FORT_OUTCOME_OK) {
        print_usage();
        return EXIT_FAILURE;
    }

    char* src = load_src(opts.filepath);
    if (src == NULL) {
        return EXIT_FAILURE;
    }

    eprintln("stage: " FMTstage, ARGstage(opts.stage));
    switch (opts.stage) {
    case STAGE_LEX: {
        tok_stream_t toks = {0};
        outcome = stage_lex(src, &toks);
        tok_stream_fini(&toks);
        exit_code = outcome == FORT_OUTCOME_OK ? EXIT_SUCCESS : EXIT_FAILURE;
        break;
    }

    case STAGE_PARSE: {
        prog_t prog = {0};
        outcome = stage_parse(src, &prog);
        exit_code = outcome == FORT_OUTCOME_OK ? EXIT_SUCCESS : EXIT_FAILURE;
        break;
    }

    case STAGE_CODEGEN: {
        asm_prog_t asm_prog = {0};
        outcome = stage_codegen(src, &asm_prog);
        asm_prog_fini(&asm_prog);
        exit_code = outcome == FORT_OUTCOME_OK ? EXIT_SUCCESS : EXIT_FAILURE;
        break;
    }

    case STAGE_COMPILE: {
        filepath_t out_file = {0};
        outcome = compute_out_file(opts.filepath, &out_file);
        if (outcome != FORT_OUTCOME_OK) {
            eprintln("could not compute output filename");
            exit_code = EXIT_FAILURE;
            break;
        }
        outcome = stage_compile(src, &out_file);
        exit_code = outcome == FORT_OUTCOME_OK ? EXIT_SUCCESS : EXIT_FAILURE;
        break;
    }
    }

    free(src);
    return exit_code;
}
