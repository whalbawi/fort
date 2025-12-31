#include "assemble.h"

#include <stddef.h>  // for NULL
#include <stdlib.h>  // for free
#include <string.h>  // for strncmp, strlen

#include "list.h"    // for list_pop_head
#include "parse.h"   // for prog_t, (anonymous struct)::(anonymous union)::(...
#include "test.h"    // for TEST_ASSERT_EQ_INT32, TEST, TEST_RUN, TEST_ASSER...

// Helper to create a program with a return statement
static prog_t make_return_prog(const char* func_name, int32_t ret_val) {
    prog_t prog = {0};
    prog.func.name.p = func_name;
    prog.func.name.len = strlen(func_name);
    prog.func.body.kind = STMT_RET;
    prog.func.body.u.ret.expr.kind = EXPR_CONST;
    prog.func.body.u.ret.expr.u.constant.val = ret_val;
    return prog;
}

TEST(simple_return_zero, {
    prog_t prog = make_return_prog("main", 0);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Should have MOV instruction (FIFO order)
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->kind, INST_MOV);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.kind, OP_IMM);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.u.imm.val, 0);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.kind, OP_REG);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.u.reg, REG_EAX);
    free(mov_inst);

    // Should have RET instruction
    inst_t* ret_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(ret_inst);
    TEST_ASSERT_EQ_INT32(ret_inst->kind, INST_RET);
    free(ret_inst);

    // Should be no more instructions
    void* next = list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_TRUE(next == NULL);

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(return_42, {
    prog_t prog = make_return_prog("main", 42);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check MOV instruction
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->kind, INST_MOV);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.kind, OP_IMM);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.u.imm.val, 42);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.kind, OP_REG);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.u.reg, REG_EAX);
    free(mov_inst);

    // Check RET instruction
    inst_t* ret_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(ret_inst);
    TEST_ASSERT_EQ_INT32(ret_inst->kind, INST_RET);
    free(ret_inst);

    // Should be no more instructions
    void* next = list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_TRUE(next == NULL);

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(return_large_number, {
    prog_t prog = make_return_prog("main", 2147483647);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check MOV instruction with INT32_MAX
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->kind, INST_MOV);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.u.imm.val, 2147483647);
    free(mov_inst);

    // Pop and free RET instruction
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(return_negative_number, {
    prog_t prog = make_return_prog("main", -100);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check MOV instruction with negative value
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->kind, INST_MOV);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.u.imm.val, -100);
    free(mov_inst);

    // Pop and free RET instruction
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(return_int32_min, {
    prog_t prog = make_return_prog("main", -2147483648);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check MOV instruction with INT32_MIN
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->kind, INST_MOV);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.src.u.imm.val, -2147483648);
    free(mov_inst);

    // Pop and free RET instruction
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(function_name_preserved, {
    prog_t prog = make_return_prog("foo", 7);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check function name is preserved
    TEST_ASSERT_EQ_SIZE(asm_prog.func.name.len, 3);
    TEST_ASSERT_TRUE(strncmp(asm_prog.func.name.p, "foo", 3) == 0);

    // Pop and free instructions
    free(list_pop_head(&asm_prog.func.insts));
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(long_function_name, {
    prog_t prog = make_return_prog("very_long_function_name", 123);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check function name is preserved
    TEST_ASSERT_EQ_SIZE(asm_prog.func.name.len, 23);
    TEST_ASSERT_TRUE(strncmp(asm_prog.func.name.p, "very_long_function_name", 23) == 0);

    // Pop and free instructions
    free(list_pop_head(&asm_prog.func.insts));
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(instruction_list_integrity, {
    prog_t prog = make_return_prog("main", 1);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Verify instruction list has exactly 2 instructions in correct order
    inst_t* first = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(first);
    TEST_ASSERT_EQ_INT32(first->kind, INST_MOV);
    free(first);

    inst_t* second = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(second);
    TEST_ASSERT_EQ_INT32(second->kind, INST_RET);
    free(second);

    // Should be no more instructions
    void* next = list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_TRUE(next == NULL);

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(mov_dst_is_eax, {
    prog_t prog = make_return_prog("test", 99);

    assembler_t* assembler = mkassembler(&prog);
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(assembler, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Verify MOV destination is always EAX
    inst_t* mov_inst = (inst_t*)list_pop_head(&asm_prog.func.insts);
    TEST_ASSERT_NONNULL(mov_inst);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.kind, OP_REG);
    TEST_ASSERT_EQ_INT32(mov_inst->u.mov.dst.u.reg, REG_EAX);
    free(mov_inst);

    // Pop and free RET instruction
    free(list_pop_head(&asm_prog.func.insts));

    asm_prog_fini(&asm_prog);
    assembler_fini(assembler);
})

TEST(null_assembler, {
    asm_prog_t asm_prog = {0};
    fort_outcome_t outcome = asm_prog_init(&asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    outcome = assembler_run(NULL, &asm_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);

    asm_prog_fini(&asm_prog);
})

TEST(null_asm_prog, {
    prog_t prog = make_return_prog("main", 0);

    assembler_t* assembler = mkassembler(&prog);
    fort_outcome_t outcome = assembler_run(assembler, NULL);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);

    assembler_fini(assembler);
})

TEST(multiple_programs, {
    prog_t prog1 = make_return_prog("main", 5);
    prog_t prog2 = make_return_prog("test", 10);

    // First program
    assembler_t* assembler1 = mkassembler(&prog1);
    asm_prog_t asm_prog1 = {0};
    fort_outcome_t outcome1 = asm_prog_init(&asm_prog1);
    TEST_ASSERT_EQ_INT32(outcome1, FORT_OUTCOME_OK);

    outcome1 = assembler_run(assembler1, &asm_prog1);
    TEST_ASSERT_EQ_INT32(outcome1, FORT_OUTCOME_OK);

    // Pop first program's MOV instruction and check value
    inst_t* mov1 = (inst_t*)list_pop_head(&asm_prog1.func.insts);
    TEST_ASSERT_NONNULL(mov1);
    TEST_ASSERT_EQ_INT32(mov1->u.mov.src.u.imm.val, 5);

    // Second program
    assembler_t* assembler2 = mkassembler(&prog2);
    asm_prog_t asm_prog2 = {0};
    fort_outcome_t outcome2 = asm_prog_init(&asm_prog2);
    TEST_ASSERT_EQ_INT32(outcome2, FORT_OUTCOME_OK);

    outcome2 = assembler_run(assembler2, &asm_prog2);
    TEST_ASSERT_EQ_INT32(outcome2, FORT_OUTCOME_OK);

    // Pop second program's MOV instruction and check value
    inst_t* mov2 = (inst_t*)list_pop_head(&asm_prog2.func.insts);
    TEST_ASSERT_NONNULL(mov2);
    TEST_ASSERT_EQ_INT32(mov2->u.mov.src.u.imm.val, 10);

    // Verify first program value hasn't changed
    TEST_ASSERT_EQ_INT32(mov1->u.mov.src.u.imm.val, 5);

    // Clean up
    free(mov1);
    free(mov2);
    free(list_pop_head(&asm_prog1.func.insts));  // RET
    free(list_pop_head(&asm_prog2.func.insts));  // RET

    asm_prog_fini(&asm_prog2);
    assembler_fini(assembler2);
    asm_prog_fini(&asm_prog1);
    assembler_fini(assembler1);
})

TEST(reuse_assembler, {
    prog_t prog = make_return_prog("main", 33);

    assembler_t* assembler = mkassembler(&prog);

    // First run
    asm_prog_t asm_prog1 = {0};
    fort_outcome_t outcome1 = asm_prog_init(&asm_prog1);
    TEST_ASSERT_EQ_INT32(outcome1, FORT_OUTCOME_OK);

    outcome1 = assembler_run(assembler, &asm_prog1);
    TEST_ASSERT_EQ_INT32(outcome1, FORT_OUTCOME_OK);

    inst_t* mov1 = (inst_t*)list_pop_head(&asm_prog1.func.insts);
    TEST_ASSERT_NONNULL(mov1);
    TEST_ASSERT_EQ_INT32(mov1->u.mov.src.u.imm.val, 33);

    // Second run with same assembler
    asm_prog_t asm_prog2 = {0};
    fort_outcome_t outcome2 = asm_prog_init(&asm_prog2);
    TEST_ASSERT_EQ_INT32(outcome2, FORT_OUTCOME_OK);

    outcome2 = assembler_run(assembler, &asm_prog2);
    TEST_ASSERT_EQ_INT32(outcome2, FORT_OUTCOME_OK);

    inst_t* mov2 = (inst_t*)list_pop_head(&asm_prog2.func.insts);
    TEST_ASSERT_NONNULL(mov2);
    TEST_ASSERT_EQ_INT32(mov2->u.mov.src.u.imm.val, 33);

    // Both programs should have independent instruction lists
    TEST_ASSERT_TRUE(mov1 != mov2);

    // Clean up
    free(mov1);
    free(mov2);
    free(list_pop_head(&asm_prog1.func.insts));  // RET
    free(list_pop_head(&asm_prog2.func.insts));  // RET

    asm_prog_fini(&asm_prog2);
    asm_prog_fini(&asm_prog1);
    assembler_fini(assembler);
})

int main(int argc, char* argv[]) {
    TEST_INIT("assemble", argc, argv);

    TEST_RUN(simple_return_zero);
    TEST_RUN(return_42);
    TEST_RUN(return_large_number);
    TEST_RUN(return_negative_number);
    TEST_RUN(return_int32_min);
    TEST_RUN(function_name_preserved);
    TEST_RUN(long_function_name);
    TEST_RUN(instruction_list_integrity);
    TEST_RUN(mov_dst_is_eax);
    TEST_RUN(null_assembler);
    TEST_RUN(null_asm_prog);
    TEST_RUN(multiple_programs);
    TEST_RUN(reuse_assembler);

    TEST_EXIT();
}
