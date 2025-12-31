#include "assemble.h"

#include <fcntl.h>   // for open, O_APPEND, O_CREAT, O_TRUNC, O_WRONLY
#include <stdio.h>   // for dprintf
#include <stdlib.h>  // for malloc, free, size_t
#include <string.h>  // for strlen
#include <unistd.h>  // for NULL, close, fsync, write, ssize_t

#include "common.h"  // for fort_outcome_t, FORT_OUTCOME_NOK_RET, FORT_OUTCO...
#include "list.h"
#include "parse.h"   // for prog_t, (anonymous struct)::(anonymous union)::(...

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

static inst_t* mkinst_mov(op_t src, op_t dst) {
    inst_t* inst = malloc(sizeof(inst_t));
    if (inst == NULL) {
        return NULL;
    }
    inst->u.mov.src = src;
    inst->u.mov.dst = dst;
    inst->kind = INST_MOV;

    return inst;
}

static inst_t* mkinst_ret(void) {
    inst_t* inst = malloc(sizeof(inst_t));
    if (inst == NULL) {
        return NULL;
    }

    inst->kind = INST_RET;

    return inst;
}

static inline fort_outcome_t gen_inst(stmt_t* body, list_t* insts) {
    switch (body->kind) {
    case STMT_RET: {
        op_t imm = {0};
        fort_outcome_t outcome = convert_expression(&body->u.ret.expr, &imm);
        FORT_OUTCOME_NOK_RET(outcome);

        inst_t* inst_mov = mkinst_mov(imm, (op_t){{{REG_EAX}}, OP_REG});
        if (inst_mov == NULL) {
            return FORT_OUTCOME_FATAL;
        }
        list_push(insts, inst_mov);

        inst_t* inst_ret = mkinst_ret();
        if (inst_ret == NULL) {
            return FORT_OUTCOME_FATAL;
        }

        list_push(insts, inst_ret);
        break;
    }

    default: {
        return FORT_OUTCOME_FATAL;
    }
    }

    return FORT_OUTCOME_OK;
}
static fort_outcome_t gen_func(func_t* func, asm_func_t* asm_func) {

    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (asm_func == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    asm_func->name = func->name;

    outcome = gen_inst(&func->body, &asm_func->insts);
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

static fort_outcome_t func_init(asm_func_t* func) {
    if (func == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    list_init(&func->insts);

    return FORT_OUTCOME_OK;
}

static void func_fini(asm_func_t* func) {
    if (func == NULL) {
        return;
    }

    LIST_ITER(&func->insts, data, void*, { free(data); })

    list_fini(&func->insts);
}

assembler_t* mkassembler(prog_t* prog) {
    assembler_t* assembler = malloc(sizeof(assembler_t));
    if (assembler == NULL) {
        return NULL;
    }

    assembler->prog = prog;

    return assembler;
}

void assembler_fini(assembler_t* assembler) {
    if (assembler == NULL) {
        return;
    }

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

fort_outcome_t asm_prog_init(asm_prog_t* prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = func_init(&prog->func);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

void asm_prog_fini(asm_prog_t* prog) {
    if (prog == NULL) {
        return;
    }

    func_fini(&prog->func);
}

static fort_outcome_t write_buf(int fd, const buf_t buf) {
    size_t len = buf.len;
    const char* p = buf.p;
    while (len > 0) {
        ssize_t nbytes = write(fd, p, len);
        if (nbytes < 0) {
            return FORT_OUTCOME_ERR;
        }

        p += nbytes;
        len -= (size_t)nbytes;
    }

    return FORT_OUTCOME_OK;
}

static inline fort_outcome_t write_newline(int fd) {
    return write_buf(fd, (buf_t){"\n", 1});
}

static fort_outcome_t emit_reg(reg_t reg, int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    switch (reg) {
    case REG_EAX: {
        const char reg_label[] = "%eax";
        outcome = write_buf(fd, (buf_t){reg_label, strlen(reg_label)});
        FORT_OUTCOME_NOK_RET(outcome);
        break;
    }

    default:
        return FORT_OUTCOME_FATAL;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_op(const op_t* op, int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    switch (op->kind) {
    case OP_IMM: {
        int nchars = dprintf(fd, "$%d", op->u.imm.val);
        if (nchars < 0) {
            return FORT_OUTCOME_ERR;
        }
        break;
    }

    case OP_REG: {
        outcome = emit_reg(op->u.reg, fd);
        FORT_OUTCOME_NOK_RET(outcome);
    } break;

    default:
        return FORT_OUTCOME_FATAL;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_inst_ret(int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    const char ret[] = "\tret\n";
    outcome = write_buf(fd, (buf_t){ret, strlen(ret)});
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_inst_mov(const mov_t* mov, int fd) {
    const char movl[] = "\tmovl ";
    fort_outcome_t outcome = write_buf(fd, (buf_t){movl, strlen(movl)});
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = emit_op(&mov->src, fd);
    FORT_OUTCOME_NOK_RET(outcome);

    const char sep[] = ", ";
    outcome = write_buf(fd, (buf_t){sep, strlen(sep)});
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = emit_op(&mov->dst, fd);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = write_newline(fd);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_inst(const list_t* insts, int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    LIST_ITER(insts, inst, inst_t*, {
        switch (inst->kind) {
        case INST_MOV: {
            outcome = emit_inst_mov(&inst->u.mov, fd);
            FORT_OUTCOME_NOK_RET(outcome);
            break;
        }

        case INST_RET: {
            outcome = emit_inst_ret(fd);
            FORT_OUTCOME_NOK_RET(outcome);
            break;
        }

        default:
            return FORT_OUTCOME_FATAL;
        }
    })

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_func(asm_func_t* func, int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    const char global_label[] = "\t.globl ";
    outcome = write_buf(fd, (buf_t){global_label, strlen(global_label)});
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = write_buf(fd, func->name);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = write_newline(fd);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = write_buf(fd, func->name);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = write_buf(fd, (buf_t){":\n", 2});
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = emit_inst(&func->insts, fd);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static fort_outcome_t emit_prog(asm_prog_t* prog, int fd) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    outcome = emit_func(&prog->func, fd);
    if (outcome != FORT_OUTCOME_OK) {
        return outcome;
    }

    const char footer[] = ".section .note.GNU-stack, \"\",@progbits\n";
    outcome = write_buf(fd, (buf_t){footer, strlen(footer)});
    if (outcome != FORT_OUTCOME_OK) {
        return outcome;
    }

    return FORT_OUTCOME_OK;
}

fort_outcome_t emit_asm(asm_prog_t* prog, const filepath_t* filepath) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    const int perms = 0644; // RW-R-R
    int fd = open(filepath->path, O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, perms);
    if (fd < 0) {
        return FORT_OUTCOME_ERR;
    }

    outcome = emit_prog(prog, fd);
    FORT_OUTCOME_NOK_RET(outcome);

    if (fsync(fd) < 0) {
        return FORT_OUTCOME_ERR;
    }

    FORT_UNUSED(close(fd));

    return FORT_OUTCOME_OK;
}
