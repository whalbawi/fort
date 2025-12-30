#include "tac.h"

#include <stdlib.h>  // for NULL, malloc, free

#include "common.h"  // for FORT_OUTCOME_FATAL, fort_outcome_t, FORT_OUTCOME_OK
#include "list.h"    // for list_push, list_fini, list_init, list_pop, list_t
#include "parse.h"   // for expr::(anonymous union)::(anonymous), expr::(ano...

struct tac {
    prog_t* prog;
    int32_t counter;
};

static void mktmp(tac_t* tac, val_t* val) {
    val->u.var.id = tac->counter;
    val->kind = TAC_VAL_VAR;
    ++tac->counter;
}

static fort_outcome_t conv_expr(tac_t* tac, expr_t* expr, val_t* val, list_t* insts) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    switch (expr->kind) {
    case EXPR_CONST: {
        val->kind = TAC_VAL_CONST;
        val->u.constant = expr->u.constant.val;
        break;
    }
    case EXPR_UNARY: {
        tac_inst_t* unop = malloc(sizeof(tac_inst_t));
        if (unop == NULL) {
            return FORT_OUTCOME_FATAL;
        }

        outcome = conv_expr(tac, expr->u.unary.expr, &unop->u.unary.src, insts);
        FORT_OUTCOME_NOK_RET(outcome);

        mktmp(tac, &unop->u.unary.dst);
        unop->u.unary.op = expr->u.unary.op == UNOP_NEGATE ? TAC_UNOP_NEGATE : TAC_UNOP_COMPLEMENT;
        unop->kind = TAC_INST_UNARY;

        *val = unop->u.unary.dst;
        list_push(insts, unop);
        break;
    }
    default:
        return FORT_OUTCOME_FATAL;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t gen_inst(tac_t* tac, stmt_t* stmt, list_t* insts) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    switch (stmt->kind) {
    case STMT_RET: {
        val_t val = {0};
        outcome = conv_expr(tac, &stmt->u.ret.expr, &val, insts);
        FORT_OUTCOME_NOK_RET(outcome);

        tac_inst_t* ret = malloc(sizeof(tac_inst_t));
        if (ret == NULL) {
            return FORT_OUTCOME_FATAL;
        }
        ret->u.ret.val = val;
        ret->kind = TAC_INST_RET;
        list_push(insts, ret);
        break;
    }
    default:
        return FORT_OUTCOME_FATAL;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t gen_func(tac_t* tac, func_t* func, tac_func_t* tac_func) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (tac_func == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = gen_inst(tac, &func->body, &tac_func->insts);
    FORT_OUTCOME_NOK_RET(outcome);

    tac_func->name = func->name;

    return FORT_OUTCOME_OK;
}

static fort_outcome_t gen_prog(tac_t* tac, prog_t* prog, tac_prog_t* tac_prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (tac_prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = gen_func(tac, &prog->func, &tac_prog->func);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static fort_outcome_t func_init(tac_func_t* func) {
    if (func == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    list_init(&func->insts);

    return FORT_OUTCOME_OK;
}

static void func_fini(tac_func_t* func) {
    if (func == NULL) {
        return;
    }

    void* data = NULL;
    while ((data = list_pop(&func->insts)) != NULL) {
        free(data);
    }

    list_fini(&func->insts);
}

tac_t* mktac(prog_t* prog) {
    tac_t* tac = malloc(sizeof(tac_t));
    if (tac == NULL) {
        return NULL;
    }

    tac->prog = prog;
    tac->counter = 0;

    return tac;
}

void tac_free(tac_t* tac) {
    free(tac);
}

fort_outcome_t tac_prog_init(tac_prog_t* prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = func_init(&prog->func);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

void tac_prog_fini(tac_prog_t* prog) {
    if (prog == NULL) {
        return;
    }

    func_fini(&prog->func);
}

fort_outcome_t tac_run(tac_t* tac, tac_prog_t* tac_prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (tac == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    if (tac_prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = gen_prog(tac, tac->prog, tac_prog);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}
