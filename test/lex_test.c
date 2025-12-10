#include "lex.h"

#include <stdbool.h>  // for bool
#include <stddef.h>   // for NULL, size_t
#include <string.h>   // for strlen, strncmp

#include "test.h"     // for TEST_ASSERT_EQ_INT32, TEST_ASSERT_TRUE, TEST

static bool lexeme_equals(const tok_t* tok, const char* expected) {
    size_t expected_len = strlen(expected);
    return tok->lexeme.len == expected_len && strncmp(tok->lexeme.p, expected, expected_len) == 0;
}

TEST(empty_input, {
    const char* src = "";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);
    tok_t* tok = toks.head.next;
    TEST_ASSERT_NONNULL(tok);
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);
    TEST_ASSERT_TRUE(tok->next == NULL);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(single_constant, {
    const char* src = "42";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_NONNULL(tok);
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "42"));
    TEST_ASSERT_NONNULL(tok->next);
    TEST_ASSERT_EQ_INT32(tok->next->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(multiple_constants, {
    const char* src = "123 456 789";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "123"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "456"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "789"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(identifier, {
    const char* src = "foo bar_baz ABC_123";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "foo"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "bar_baz"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "ABC_123"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(punctuation, {
    const char* src = "(){};";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_PAREN);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "("));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_PAREN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_SEMICOLON);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(whitespace_handling, {
    const char* src = "  \t\n  42  \n\n  foo  \t";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "42"));
    TEST_ASSERT_EQ_INT32(tok->line, 2);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "foo"));
    TEST_ASSERT_EQ_INT32(tok->line, 4);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(simple_function, {
    const char* src = "i32 main() { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_KEYWORD_I32);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "i32"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "main"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_PAREN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_PAREN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_KEYWORD_RETURN);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "return"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "0"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_SEMICOLON);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(invalid_constant_with_letter, {
    const char* src = "123abc";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);
    tok_t* tok = toks.head.next;
    TEST_ASSERT_NONNULL(tok);
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_ERROR);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(line_tracking, {
    const char* src = "foo\nbar\n\nbaz";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 1);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 2);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 4);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(mixed_tokens, {
    const char* src = "x = 42 + y;";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_ERR);
    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "x"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_ERROR);  // '=' not yet supported

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(single_line_comment, {
    const char* src = "42 // this is a comment\n99";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "42"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "99"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(comment_at_start, {
    const char* src = "// comment at start\nfoo";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "foo"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(comment_at_end, {
    const char* src = "bar // comment at end";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "bar"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(multiple_comments, {
    const char* src = "// first comment\nx // second\n// third\ny";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "x"));
    TEST_ASSERT_EQ_INT32(tok->line, 2);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "y"));
    TEST_ASSERT_EQ_INT32(tok->line, 4);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(comment_with_code_like_content, {
    const char* src = "// int x = 42; return foo;\nactual";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "actual"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(function_with_comments, {
    const char* src = "i32 main() {\n"
                      "    // return value\n"
                      "    return 0; // success\n"
                      "}";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_KEYWORD_I32);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "main"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_PAREN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_PAREN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_OPEN_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_KEYWORD_RETURN);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "0"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_SEMICOLON);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CLOSE_BRACE);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

TEST(empty_comment, {
    const char* src = "foo //\nbar";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};
    fort_outcome_t outcome = lexer_run(lexer, &toks);

    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    tok_t* tok = toks.head.next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "foo"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "bar"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    tok_stream_fini(&toks);
    lexer_fini(lexer);
})

int main(int argc, char* argv[]) {
    TEST_INIT("lex", argc, argv);

    TEST_RUN(empty_input);
    TEST_RUN(single_constant);
    TEST_RUN(multiple_constants);
    TEST_RUN(identifier);
    TEST_RUN(punctuation);
    TEST_RUN(whitespace_handling);
    TEST_RUN(simple_function);
    TEST_RUN(invalid_constant_with_letter);
    TEST_RUN(line_tracking);
    TEST_RUN(mixed_tokens);
    TEST_RUN(single_line_comment);
    TEST_RUN(comment_at_start);
    TEST_RUN(comment_at_end);
    TEST_RUN(multiple_comments);
    TEST_RUN(comment_with_code_like_content);
    TEST_RUN(function_with_comments);
    TEST_RUN(empty_comment);

    TEST_EXIT();
}
