#ifndef FORT_PARSE_H
#define FORT_PARSE_H

#include <inttypes.h>  // for int32_t

#include "common.h"    // for buf_t, fort_outcome_t
#include "lex.h"       // for tok_stream_t

typedef struct parser parser_t;

typedef enum {
    EXPR_CONST,
    EXPR_UNARY,
} expr_kind_t;

typedef enum {
    UNOP_COMPLEMENT,
    UNOP_NEGATE,
} unop_t;

typedef struct expr {
    union {
        struct {
            int32_t val;
        } constant;
        struct {
            unop_t op;
            struct expr* expr;
        } unary;
    } u;
    expr_kind_t kind;
} expr_t;

typedef enum {
    STMT_RET,
} stmt_kind_t;

typedef struct {
    union {
        struct {
            expr_t expr;
        } ret;
    } u;
    stmt_kind_t kind;
} stmt_t;

typedef struct {
    buf_t name;
    stmt_t body;
} func_t;

typedef struct {
    func_t func;
} prog_t;

parser_t* mkparser(tok_stream_t* toks);

void parser_fini(parser_t* parser);

fort_outcome_t parser_run(parser_t* parser, prog_t* prog);

void prog_fini(prog_t* prog);

void ast_print(const prog_t* prog);

#endif // FORT_PARSE_H
