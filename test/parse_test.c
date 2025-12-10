#include "parse.h"

#include <stddef.h>  // for NULL
#include <string.h>  // for strlen

#include "lex.h"     // for lexer_fini, lexer_run, mklexer, tok_stream_fini
#include "test.h"    // for TEST_ASSERT_EQ_INT32, TEST, TEST_RUN, TEST_EXIT

TEST(simple_program, {
    const char* src = "i32 main(void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.kind, STMT_RET);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.kind, EXPR_CONST);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 0);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(return_42, {
    const char* src = "i32 main(void) { return 42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.kind, STMT_RET);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.kind, EXPR_CONST);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 42);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(return_large_number, {
    const char* src = "i32 main(void) { return 2147483647; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.kind, STMT_RET);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.kind, EXPR_CONST);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 2147483647);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(overflow_constant, {
    const char* src = "i32 main(void) { return 2147483648; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(with_whitespace, {
    const char* src = "  i32   main  (  void  )  {  return   100  ;  }  ";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 100);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(with_comments, {
    const char* src = "// This is a comment\n"
                      "i32 main(void) { // main function\n"
                      "    return 42; // return value\n"
                      "}\n";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 42);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(multiline_program, {
    const char* src = "i32 main(void) {\n"
                      "    return 7;\n"
                      "}\n";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    TEST_ASSERT_EQ_INT32(prog.func.body.u.ret.expr.u.constant.val, 7);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_return_keyword, {
    const char* src = "i32 main(void) { 42; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_semicolon, {
    const char* src = "i32 main(void) { return 42 }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);  // propagates outcome from expect

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_expression, {
    const char* src = "i32 main(void) { return ; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_open_brace, {
    const char* src = "i32 main(void) return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_close_brace, {
    const char* src = "i32 main(void) { return 0;";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_void_keyword, {
    const char* src = "i32 main() { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(wrong_return_type, {
    const char* src = "void main(void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(missing_function_name, {
    const char* src = "i32 (void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(empty_input, {
    const char* src = "";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(parser, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(null_parser, {
    prog_t prog = {0};
    fort_outcome_t outcome = parser_run(NULL, &prog);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);
})

TEST(null_prog, {
    const char* src = "i32 main(void) { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t lex_outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(lex_outcome, FORT_OUTCOME_OK);
    parser_t* parser = mkparser(&toks);

    fort_outcome_t outcome = parser_run(parser, NULL);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_FATAL);

    parser_fini(parser);
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

int main(int argc, char* argv[]) {
    TEST_INIT("parse", argc, argv);

    TEST_RUN(simple_program);
    TEST_RUN(return_42);
    TEST_RUN(return_large_number);
    TEST_RUN(overflow_constant);
    TEST_RUN(with_whitespace);
    TEST_RUN(with_comments);
    TEST_RUN(multiline_program);
    TEST_RUN(missing_return_keyword);
    TEST_RUN(missing_semicolon);
    TEST_RUN(missing_expression);
    TEST_RUN(missing_open_brace);
    TEST_RUN(missing_close_brace);
    TEST_RUN(missing_void_keyword);
    TEST_RUN(wrong_return_type);
    TEST_RUN(missing_function_name);
    TEST_RUN(empty_input);
    TEST_RUN(null_parser);
    TEST_RUN(null_prog);

    TEST_EXIT();
}
