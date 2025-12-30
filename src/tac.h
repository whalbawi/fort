#ifndef FORT_TAC_H
#define FORT_TAC_H

#include <stdint.h>  // for int32_t

#include "common.h"  // for fort_outcome_t, buf_t
#include "list.h"    // for list_t
#include "parse.h"   // for prog_t

typedef struct tac tac_t;

typedef enum {
    TAC_VAL_CONST,
    TAC_VAL_VAR,
} tac_val_kind_t;

typedef struct {
    union {
        int32_t constant;
        struct {
            int32_t id;
        } var;
    } u;
    tac_val_kind_t kind;
} val_t;

typedef enum {
    TAC_INST_RET,
    TAC_INST_UNARY,
} tac_inst_kind_t;

typedef enum {
    TAC_UNOP_COMPLEMENT,
    TAC_UNOP_NEGATE,
} tac_unop_t;

typedef struct tac_inst {
    union {
        struct {
            val_t val;
        } ret;
        struct {
            tac_unop_t op;
            val_t src;
            val_t dst;
        } unary;
    } u;
    tac_inst_kind_t kind;
} tac_inst_t;

typedef struct {
    buf_t name;
    list_t insts;
} tac_func_t;

typedef struct {
    tac_func_t func;
} tac_prog_t;

tac_t* mktac(prog_t* prog);
void tac_free(tac_t* tac);

fort_outcome_t tac_prog_init(tac_prog_t* prog);
void tac_prog_fini(tac_prog_t* prog);

fort_outcome_t tac_run(tac_t* tac, tac_prog_t* prog);

#endif // FORT_TAC_H
