#include "tac.h"

#include <stddef.h>  // for NULL
#include <stdlib.h>  // for free
#include <string.h>  // for strlen

#include "lex.h"     // for lexer_fini, lexer_run, mklexer, tok_stream_fini
#include "parse.h"   // for mkparser, parser_fini, parser_run, prog_fini
#include "test.h"    // for TEST_ASSERT_EQ_INT32, TEST, TEST_RUN, TEST_ASSER...

TEST(create_tac, {
    const char* src = "i32 main(void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    TEST_ASSERT_NONNULL(tac);

    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(simple_constant, {
    const char* src = "i32 main(void) { return 42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Check that we have exactly one instruction (return)
    void* inst = list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(inst);
    tac_inst_t* ret_inst = (tac_inst_t*)inst;
    TEST_ASSERT_EQ_INT32(ret_inst->kind, TAC_INST_RET);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.kind, TAC_VAL_CONST);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.u.constant, 42);

    // Should be no more instructions
    void* next = list_pop(&tac_prog.func.insts);
    TEST_ASSERT_TRUE(next == NULL);

    free(inst);
    tac_prog_fini(&tac_prog);
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(unary_complement, {
    const char* src = "i32 main(void) { return ~42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Should have 2 instructions: unary op, then return
    tac_inst_t* ret_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(ret_inst);
    TEST_ASSERT_EQ_INT32(ret_inst->kind, TAC_INST_RET);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.kind, TAC_VAL_VAR);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.u.var.id, 0);  // First temp is t0

    tac_inst_t* unary_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(unary_inst);
    TEST_ASSERT_EQ_INT32(unary_inst->kind, TAC_INST_UNARY);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.op, TAC_UNOP_COMPLEMENT);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.src.kind, TAC_VAL_CONST);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.src.u.constant, 42);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.dst.kind, TAC_VAL_VAR);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.dst.u.var.id, 0);

    void* next = list_pop(&tac_prog.func.insts);
    TEST_ASSERT_TRUE(next == NULL);

    free(ret_inst);
    free(unary_inst);
    tac_prog_fini(&tac_prog);
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(unary_negate, {
    const char* src = "i32 main(void) { return -100; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tac_inst_t* ret_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(ret_inst);
    TEST_ASSERT_EQ_INT32(ret_inst->kind, TAC_INST_RET);

    tac_inst_t* unary_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(unary_inst);
    TEST_ASSERT_EQ_INT32(unary_inst->kind, TAC_INST_UNARY);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.op, TAC_UNOP_NEGATE);
    TEST_ASSERT_EQ_INT32(unary_inst->u.unary.src.u.constant, 100);

    free(ret_inst);
    free(unary_inst);
    tac_prog_fini(&tac_prog);
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(nested_unary, {
    const char* src = "i32 main(void) { return ~-42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Should have 3 instructions: negate, complement, return
    tac_inst_t* ret_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(ret_inst);
    TEST_ASSERT_EQ_INT32(ret_inst->kind, TAC_INST_RET);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.u.var.id, 1);  // Result is t1

    tac_inst_t* comp_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(comp_inst);
    TEST_ASSERT_EQ_INT32(comp_inst->kind, TAC_INST_UNARY);
    TEST_ASSERT_EQ_INT32(comp_inst->u.unary.op, TAC_UNOP_COMPLEMENT);
    TEST_ASSERT_EQ_INT32(comp_inst->u.unary.src.kind, TAC_VAL_VAR);
    TEST_ASSERT_EQ_INT32(comp_inst->u.unary.src.u.var.id, 0);  // Source is t0
    TEST_ASSERT_EQ_INT32(comp_inst->u.unary.dst.u.var.id, 1);  // Dest is t1

    tac_inst_t* neg_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_NONNULL(neg_inst);
    TEST_ASSERT_EQ_INT32(neg_inst->kind, TAC_INST_UNARY);
    TEST_ASSERT_EQ_INT32(neg_inst->u.unary.op, TAC_UNOP_NEGATE);
    TEST_ASSERT_EQ_INT32(neg_inst->u.unary.src.kind, TAC_VAL_CONST);
    TEST_ASSERT_EQ_INT32(neg_inst->u.unary.src.u.constant, 42);
    TEST_ASSERT_EQ_INT32(neg_inst->u.unary.dst.u.var.id, 0);  // Dest is t0

    free(ret_inst);
    free(comp_inst);
    free(neg_inst);
    tac_prog_fini(&tac_prog);
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(double_negate, {
    const char* src = "i32 main(void) { return - -8; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Should have 3 instructions: negate, negate, return
    tac_inst_t* ret_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_EQ_INT32(ret_inst->u.ret.val.u.var.id, 1);

    tac_inst_t* neg2_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_EQ_INT32(neg2_inst->u.unary.op, TAC_UNOP_NEGATE);
    TEST_ASSERT_EQ_INT32(neg2_inst->u.unary.src.u.var.id, 0);
    TEST_ASSERT_EQ_INT32(neg2_inst->u.unary.dst.u.var.id, 1);

    tac_inst_t* neg1_inst = (tac_inst_t*)list_pop(&tac_prog.func.insts);
    TEST_ASSERT_EQ_INT32(neg1_inst->u.unary.op, TAC_UNOP_NEGATE);
    TEST_ASSERT_EQ_INT32(neg1_inst->u.unary.src.u.constant, 8);
    TEST_ASSERT_EQ_INT32(neg1_inst->u.unary.dst.u.var.id, 0);

    free(ret_inst);
    free(neg2_inst);
    free(neg1_inst);
    tac_prog_fini(&tac_prog);
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(counter_resets, {
    // First compilation
    const char* src1 = "i32 main(void) { return ~1; }";
    lexer_t* lexer1 = mklexer(src1, strlen(src1));
    tok_stream_t toks1 = {0};
    fort_outcome_t lex_outcome1 = lexer_run(lexer1, &toks1);
    TEST_ASSERT_EQ_INT32(lex_outcome1, FORT_OUTCOME_OK);
    parser_t* parser1 = mkparser(&toks1);
    prog_t prog1 = {0};
    fort_outcome_t parse_outcome1 = parser_run(parser1, &prog1);
    TEST_ASSERT_EQ_INT32(parse_outcome1, FORT_OUTCOME_OK);

    tac_t* tac1 = mktac(&prog1);
    tac_prog_t tac_prog1 = {0};
    fort_outcome_t init_outcome1 = tac_prog_init(&tac_prog1);
    TEST_ASSERT_EQ_INT32(init_outcome1, FORT_OUTCOME_OK);
    fort_outcome_t run_outcome1 = tac_run(tac1, &tac_prog1);
    TEST_ASSERT_EQ_INT32(run_outcome1, FORT_OUTCOME_OK);

    tac_inst_t* ret1 = (tac_inst_t*)list_pop(&tac_prog1.func.insts);
    tac_inst_t* unary1 = (tac_inst_t*)list_pop(&tac_prog1.func.insts);
    TEST_ASSERT_EQ_INT32(unary1->u.unary.dst.u.var.id, 0);  // First temp is t0

    free(ret1);
    free(unary1);
    tac_prog_fini(&tac_prog1);
    tac_free(tac1);
    prog_fini(&prog1);
    parser_fini(parser1);
    tok_stream_fini(&toks1);
    lexer_fini(lexer1);

    // Second compilation - counter should reset
    const char* src2 = "i32 main(void) { return -2; }";
    lexer_t* lexer2 = mklexer(src2, strlen(src2));
    tok_stream_t toks2 = {0};
    fort_outcome_t lex_outcome2 = lexer_run(lexer2, &toks2);
    TEST_ASSERT_EQ_INT32(lex_outcome2, FORT_OUTCOME_OK);
    parser_t* parser2 = mkparser(&toks2);
    prog_t prog2 = {0};
    fort_outcome_t parse_outcome2 = parser_run(parser2, &prog2);
    TEST_ASSERT_EQ_INT32(parse_outcome2, FORT_OUTCOME_OK);

    tac_t* tac2 = mktac(&prog2);
    tac_prog_t tac_prog2 = {0};
    fort_outcome_t init_outcome2 = tac_prog_init(&tac_prog2);
    TEST_ASSERT_EQ_INT32(init_outcome2, FORT_OUTCOME_OK);
    fort_outcome_t run_outcome2 = tac_run(tac2, &tac_prog2);
    TEST_ASSERT_EQ_INT32(run_outcome2, FORT_OUTCOME_OK);

    tac_inst_t* ret2 = (tac_inst_t*)list_pop(&tac_prog2.func.insts);
    tac_inst_t* unary2 = (tac_inst_t*)list_pop(&tac_prog2.func.insts);
    TEST_ASSERT_EQ_INT32(unary2->u.unary.dst.u.var.id, 0);  // Counter reset, first temp is t0

    free(ret2);
    free(unary2);
    tac_prog_fini(&tac_prog2);
    tac_free(tac2);
    prog_fini(&prog2);
    parser_fini(parser2);
    tok_stream_fini(&toks2);
    lexer_fini(lexer2);
})

TEST(null_tac, {
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(NULL, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);

    tac_prog_fini(&tac_prog);
})

TEST(null_prog, {
    const char* src = "i32 main(void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);

    fort_outcome_t outcome = tac_run(tac, NULL);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);

    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(memory_leak_check, {
    // This test generates multiple TAC instructions that are heap-allocated.
    // If tac_prog_fini() doesn't properly free the instruction list,
    // memory sanitizers (ASAN/MSAN) will detect the leak.
    const char* src = "i32 main(void) { return ~-~-42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);

    parser_t* parser = mkparser(&toks);
    prog_t prog = {0};
    fort_outcome_t parse_outcome = parser_run(parser, &prog);
    TEST_ASSERT_EQ_INT32(parse_outcome, FORT_OUTCOME_OK);

    tac_t* tac = mktac(&prog);
    tac_prog_t tac_prog = {0};
    fort_outcome_t init_outcome = tac_prog_init(&tac_prog);
    TEST_ASSERT_EQ_INT32(init_outcome, FORT_OUTCOME_OK);

    fort_outcome_t outcome = tac_run(tac, &tac_prog);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // This test creates 5 heap-allocated instructions:
    // 1. negate(42) -> t0
    // 2. complement(t0) -> t1
    // 3. negate(t1) -> t2
    // 4. complement(t2) -> t3
    // 5. return t3
    //
    // If tac_prog_fini() doesn't free these instructions,
    // we'll leak 5 * sizeof(tac_inst_t) bytes.

    tac_prog_fini(&tac_prog);  // This MUST free all instructions
    tac_free(tac);
    prog_fini(&prog);
    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

int main(int argc, char* argv[]) {
    TEST_INIT("tac", argc, argv);

    TEST_RUN(create_tac);
    TEST_RUN(simple_constant);
    TEST_RUN(unary_complement);
    TEST_RUN(unary_negate);
    TEST_RUN(nested_unary);
    TEST_RUN(double_negate);
    TEST_RUN(counter_resets);
    TEST_RUN(null_tac);
    TEST_RUN(null_prog);
    TEST_RUN(memory_leak_check);

    TEST_EXIT();
}
