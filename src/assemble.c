#include "assemble.h"

#include <stdlib.h>

#include "common.h"
#include "parse.h"

struct assembler {
    prog_t* prog;
};

static fort_outcome_t convert_expression(expr_t* expr, op_t* op) {
    switch (expr->kind) {
    case EXPR_CONST: {
        op->u.imm.val = expr->u.constant.val;
        op->kind = OP_IMM;

        return FORT_OUTCOME_OK;
    }
    default:
        return FORT_OUTCOME_FATAL;
    }
}

static inline fort_outcome_t gen_inst(stmt_t* body, inst_t** inst) {
    switch (body->kind) {
    case STMT_RET: {
        op_t imm = {0};
        fort_outcome_t outcome = convert_expression(&body->u.ret.expr, &imm);
        FORT_OUTCOME_NOK_RET(outcome);
        inst_t* inst_mov = malloc(sizeof(inst_t));
        inst_mov->u.mov.src = imm;
        inst_mov->u.mov.dst = (op_t){{{REG_EAX}}, OP_REG};
        inst_mov->kind = INST_MOV;

        inst_t* inst_ret = malloc(sizeof(inst_t));
        inst_ret->kind = INST_RET;
        inst_ret->next = NULL;
        inst_mov->next = inst_ret;

        *inst = inst_mov;

        return FORT_OUTCOME_OK;
    }
    default:
        return FORT_OUTCOME_FATAL;
    }
}
static fort_outcome_t gen_func(func_t* func, asm_func_t* asm_func) {

    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (asm_func == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    asm_func->name = func->name;

    outcome = gen_inst(&func->body, &asm_func->inst);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static fort_outcome_t gen_prog(prog_t* prog, asm_prog_t* asm_prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (asm_prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = gen_func(&prog->func, &asm_prog->func);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

assembler_t* mkassembler(prog_t* prog) {
    assembler_t* assembler = malloc(sizeof(assembler_t));
    assembler->prog = prog;

    return assembler;
}

void assembler_fini(assembler_t* assembler) {
    free(assembler);
}

fort_outcome_t assembler_run(assembler_t* assembler, asm_prog_t* asm_prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (assembler == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    if (asm_prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = gen_prog(assembler->prog, asm_prog);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

void asm_prog_fini(asm_prog_t* asm_prog) {
    inst_t* inst = asm_prog->func.inst;
    while (inst != NULL) {
        inst_t* next = inst->next;
        free(inst);
        inst = next;
    }
}
