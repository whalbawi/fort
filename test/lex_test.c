#include "lex.h"

#include <stdbool.h>  // for bool
#include <stddef.h>   // for NULL, size_t
#include <string.h>   // for strlen, strncmp

#include "test.h"     // for TEST_ASSERT_EQ_INT32, TEST_ASSERT_TRUE, TEST

static bool lexeme_equals(const tok_t* tok, const char* expected) {
    size_t expected_len = strlen(expected);
    return tok->lexeme.len == expected_len && strncmp(tok->lexeme.p, expected, expected_len) == 0;
}

TEST(test_empty_input, {
    const char* src = "";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    TEST_ASSERT_NONNULL(tokens);
    TEST_ASSERT_EQ_INT32(tokens->type, TOKT_EOF);
    TEST_ASSERT_TRUE(tokens->next == NULL);

    lexer_fini(lexer);
})

TEST(test_single_constant, {
    const char* src = "42";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    TEST_ASSERT_NONNULL(tokens);
    TEST_ASSERT_EQ_INT32(tokens->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tokens, "42"));
    TEST_ASSERT_NONNULL(tokens->next);
    TEST_ASSERT_EQ_INT32(tokens->next->type, TOKT_EOF);

    lexer_fini(lexer);
})

TEST(test_multiple_constants, {
    const char* src = "123 456 789";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
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

    lexer_fini(lexer);
})

TEST(test_identifier, {
    const char* src = "foo bar_baz ABC_123";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
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

    lexer_fini(lexer);
})

TEST(test_punctuation, {
    const char* src = "(){};";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
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

    lexer_fini(lexer);
})

TEST(test_whitespace_handling, {
    const char* src = "  \t\n  42  \n\n  foo  \t";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_CONSTANT);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "42"));
    TEST_ASSERT_EQ_INT32(tok->line, 2);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "foo"));
    TEST_ASSERT_EQ_INT32(tok->line, 4);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_EOF);

    lexer_fini(lexer);
})

TEST(test_simple_function, {
    const char* src = "i32 main() { return 0; }";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
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

    lexer_fini(lexer);
})

TEST(test_invalid_constant_with_letter, {
    const char* src = "123abc";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    TEST_ASSERT_NONNULL(tokens);
    TEST_ASSERT_EQ_INT32(tokens->type, TOKT_ERROR);

    lexer_fini(lexer);
})

TEST(test_line_tracking, {
    const char* src = "foo\nbar\n\nbaz";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 1);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 2);

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_EQ_INT32(tok->line, 4);

    lexer_fini(lexer);
})

TEST(test_mixed_tokens, {
    const char* src = "x = 42 + y;";
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_t* tokens = lexer_run(lexer);

    tok_t* tok = tokens;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_IDENTIFIER);
    TEST_ASSERT_TRUE(lexeme_equals(tok, "x"));

    tok = tok->next;
    TEST_ASSERT_EQ_INT32(tok->type, TOKT_ERROR);  // '=' not yet supported

    lexer_fini(lexer);
})

TEST(test_idempotent_lexer_run, {
    const char* src = "42 foo";
    lexer_t* lexer = mklexer(src, strlen(src));

    // First call
    tok_t* tokens1 = lexer_run(lexer);
    TEST_ASSERT_NONNULL(tokens1);
    TEST_ASSERT_EQ_INT32(tokens1->type, TOKT_CONSTANT);

    // Second call - should return same tokens
    tok_t* tokens2 = lexer_run(lexer);
    TEST_ASSERT_TRUE(tokens1 == tokens2);

    lexer_fini(lexer);
})

int main(int argc, char* argv[]) {
    TEST_INIT("lex", argc, argv);

    TEST_RUN(test_empty_input);
    TEST_RUN(test_single_constant);
    TEST_RUN(test_multiple_constants);
    TEST_RUN(test_identifier);
    TEST_RUN(test_punctuation);
    TEST_RUN(test_whitespace_handling);
    TEST_RUN(test_simple_function);
    TEST_RUN(test_invalid_constant_with_letter);
    TEST_RUN(test_line_tracking);
    TEST_RUN(test_mixed_tokens);
    TEST_RUN(test_idempotent_lexer_run);

    TEST_EXIT();
}
