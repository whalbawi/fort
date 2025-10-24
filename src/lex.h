#ifndef FORT_LEX_H
#define FORT_LEX_H

#include <stdbool.h>  // for bool
#include <stddef.h>   // for size_t
#include <stdint.h>   // for uint32_t

typedef struct lexer lexer_t;

typedef enum {
    TOKT_IDENTIFIER,
    TOKT_CONSTANT,
    TOKT_KEYWORD_I32,
    TOKT_KEYWORD_VOID,
    TOKT_KEYWORD_RETURN,
    TOKT_OPEN_PAREN,
    TOKT_CLOSE_PAREN,
    TOKT_OPEN_BRACE,
    TOKT_CLOSE_BRACE,
    TOKT_SEMICOLON,
    TOKT_EOF,
    TOKT_ERROR,
} tokt_t;

typedef struct tok {
    tokt_t type;
    uint32_t line;
    struct {
        const char* p;
        size_t len;
    } lexeme;
    struct tok* next;
} tok_t;

typedef struct {
    tok_t head;
    bool err;
} tok_stream_t;

lexer_t* mklexer(const char* src, size_t len);

void lexer_fini(lexer_t* lexer);

tok_stream_t lexer_run(lexer_t* lexer);

void tok_stream_fini(tok_stream_t* tok);

#endif // FORT_LEX_H
