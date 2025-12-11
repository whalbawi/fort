#ifndef FORT_ASSEMBLE_H
#define FORT_ASSEMBLE_H

#include <stdint.h>

#include "common.h"
#include "parse.h"

typedef struct assembler assembler_t;

typedef enum {
    REG_EAX,
} reg_t;

typedef enum {
    OP_IMM,
    OP_REG,
} op_kind_t;

typedef struct {
    union {
        struct {
            int32_t val;
        } imm;
        reg_t reg;
    } u;
    op_kind_t kind;
} op_t;

typedef enum {
    INST_MOV,
    INST_RET,
} inst_kind_t;

typedef struct inst {
    union {
        struct {
            op_t src;
            op_t dst;
        } mov;
    } u;
    inst_kind_t kind;
    struct inst* next;
} inst_t;

typedef struct {
    buf_t name;
    inst_t* inst;
} asm_func_t;

typedef struct {
    asm_func_t func;
} asm_prog_t;

assembler_t* mkassembler(prog_t* prog);

void assembler_fini(assembler_t* assembler);

fort_outcome_t assembler_run(assembler_t* assembler, asm_prog_t* asm_prog);

void asm_prog_fini(asm_prog_t* asm_prog);

#endif // FORT_ASSEMBLE_H
