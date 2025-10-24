#include "lex.h"

#include <stdbool.h>  // for bool, true
#include <stddef.h>   // for NULL, size_t
#include <stdint.h>   // for uint32_t
#include <stdlib.h>   // for free, malloc
#include <string.h>   // for strncmp

#include "common.h"   // for FORT_UNUSED, NELEM

typedef struct {
    const char* lexeme;
    tokt_t type;
} keyword_t;

static const keyword_t KEYWORDS[] = {
    {"i32", TOKT_KEYWORD_I32},
    {"void", TOKT_KEYWORD_VOID},
    {"return", TOKT_KEYWORD_RETURN},
};

struct lexer {
    const char* src;
    size_t pos;
    uint32_t line;
    tok_t head;
};

static inline bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}

static inline bool is_alpha(const char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool is_alphanum(const char c) {
    return is_alpha(c) || is_digit(c);
}

static inline bool is_whitespace(const char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static char peek(const lexer_t* lexer) {
    return lexer->src[lexer->pos];
}

static char peek_next(const lexer_t* lexer) {
    if (lexer->src[lexer->pos] == '\0') {
        return '\0';
    }

    return lexer->src[lexer->pos + 1];
}

static char advance(lexer_t* lexer) {
    const char c = peek(lexer);
    lexer->pos++;

    return c;
}

static void seek_lexeme(lexer_t* lexer) {
    for (;;) {
        char c = peek(lexer);
        if (is_whitespace(c)) {
            lexer->src++;
            if (c == '\n') {
                lexer->line++;
            }
        } else if (c == '/') {
            if (peek_next(lexer) == '/') {
                while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                    lexer->src++;
                }
            } else {
                return;
            }
        } else {
            return;
        }
    }
}

static tok_t mktok(const lexer_t* lexer, tokt_t type) {
    return (tok_t){type, lexer->line, {lexer->src, lexer->pos}, NULL};
}

static tok_t mkerr(const lexer_t* lexer) {
    return mktok(lexer, TOKT_ERROR);
}

static tok_t mkconst(lexer_t* lexer) {
    while (is_digit(peek(lexer))) {
        FORT_UNUSED(advance(lexer));
    }

    if (is_alpha(peek(lexer))) {
        return mkerr(lexer);
    }

    return mktok(lexer, TOKT_CONSTANT);
}

static tok_t mkident(lexer_t* lexer) {
    while (is_alphanum(peek(lexer))) {
        FORT_UNUSED(advance(lexer));
    }

    for (uint32_t i = 0; i < NELEM(KEYWORDS); ++i) {
        const keyword_t* const keyword = KEYWORDS + i;
        if (strncmp(lexer->src, keyword->lexeme, lexer->pos) == 0) {
            return mktok(lexer, keyword->type);
        }
    }

    return mktok(lexer, TOKT_IDENTIFIER);
}

static void consume_lexeme(lexer_t* lexer) {
    lexer->src += lexer->pos;
    lexer->pos = 0;
}

static tok_t lexer_next(lexer_t* lexer) {
    seek_lexeme(lexer);
    const char c = advance(lexer);
    tok_t tok;

    switch (c) {
    case '\0':
        tok = mktok(lexer, TOKT_EOF);
        break;
    case '(':
        tok = mktok(lexer, TOKT_OPEN_PAREN);
        break;
    case ')':
        tok = mktok(lexer, TOKT_CLOSE_PAREN);
        break;
    case '{':
        tok = mktok(lexer, TOKT_OPEN_BRACE);
        break;
    case '}':
        tok = mktok(lexer, TOKT_CLOSE_BRACE);
        break;
    case ';':
        tok = mktok(lexer, TOKT_SEMICOLON);
        break;
    default:
        if (is_digit(c)) {
            tok = mkconst(lexer);
            break;
        } else if (is_alpha(c)) {
            tok = mkident(lexer);
            break;
        } else {
            tok = mkerr(lexer);
            break;
        }
    }

    consume_lexeme(lexer);
    return tok;
}

lexer_t* mklexer(const char* const src, const size_t len) {
    lexer_t* lexer = malloc(sizeof(lexer_t));

    lexer->src = src;
    FORT_UNUSED(len);
    lexer->pos = 0;
    lexer->line = 1;
    lexer->head = (tok_t){0};

    return lexer;
}

void lexer_fini(lexer_t* lexer) {
    free(lexer);
}

tok_stream_t lexer_run(lexer_t* lexer) {
    tok_stream_t stream = {0};
    tok_t* ip = &stream.head;

    for (;;) {
        tok_t* tok = malloc(sizeof(tok_t));
        *tok = lexer_next(lexer);
        ip->next = tok;
        ip = ip->next;

        if (tok->type == TOKT_ERROR) {
            stream.err = true;
        }

        if (tok->type == TOKT_EOF) {
            break;
        }
    }

    return stream;
}

void tok_stream_fini(tok_stream_t* toks) {
    tok_t* tok = toks->head.next;
    while (tok != NULL) {
        tok_t* next = tok->next;
        free(tok);
        tok = next;
    }
}
