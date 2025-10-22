#include "lex.h"

#include <stdbool.h>  // for bool
#include <stddef.h>   // for NULL, size_t
#include <stdint.h>   // for uint32_t
#include <stdlib.h>   // for free, malloc

#include "common.h"   // for FORT_UNUSED

struct lexer {
    const char* src;
    size_t pos;
    uint32_t line;
    tok_t head;
};

static bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(const char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_whitespace(const char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static char peek(const lexer_t* lexer) {
    return lexer->src[lexer->pos];
}

static char advance(lexer_t* lexer) {
    const char c = peek(lexer);
    lexer->pos++;

    return c;
}

static void skip_whitespace(lexer_t* lexer) {
    char c = '\0';

    while ((c = peek(lexer)) != '\0') {
        if (is_whitespace(c)) {
            if (c == '\n') {
                lexer->line++;
            }
            lexer->src++;
        } else {
            break;
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
    while (is_alpha(peek(lexer))) {
        FORT_UNUSED(advance(lexer));
    }

    return mktok(lexer, TOKT_IDENTIFIER);
}

static void consume_lexeme(lexer_t* lexer) {
    lexer->src += lexer->pos;
    lexer->pos = 0;
}

static tok_t lexer_next(lexer_t* lexer) {
    skip_whitespace(lexer);
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
    tok_t* tok = lexer->head.next;
    while (tok != NULL) {
        tok_t* next = tok->next;
        free(tok);
        tok = next;
    }

    free(lexer);
}

tok_t* lexer_run(lexer_t* lexer) {
    if (lexer->head.next != NULL) {
        return lexer->head.next;
    }

    tok_t* ip = &lexer->head;
    for (;;) {
        tok_t* tok = malloc(sizeof(tok_t));
        *tok = lexer_next(lexer);
        ip->next = tok;
        ip = ip->next;

        if (tok->type == TOKT_EOF) {
            break;
        }
    }

    return lexer->head.next;
}
