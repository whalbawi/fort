# Test configuration

# Lexer tests
add_executable(lex_test
    lex_test.c
    ${FORT_SRC_DIR}/lex.c
)

target_include_directories(lex_test PRIVATE ${FORT_SRC_DIR})

add_test(NAME lex_test COMMAND lex_test)
