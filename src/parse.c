#include "parse.h"

#include <inttypes.h>  // for int32_t, INT32_MAX
#include <stdlib.h>    // for NULL, free, malloc, size_t

#include "common.h"    // for FORT_OUTCOME_NOK_RET, fort_outcome_t, FORT_OUT...
#include "lex.h"       // for tok_stream_t, tok_t, TOKT_TILDE, TOKT_CLOSE_PAREN

typedef enum {
    AST_NODE_PROG,
    AST_NODE_FUNC,
    AST_NODE_STMT,
    AST_NODE_EXPR,
} ast_node_kind_t;

typedef struct {
    union {
        prog_t prog;
        func_t func;
        stmt_t stmt;
        expr_t expr;
    } u;
    ast_node_kind_t kind;
} ast_node_t;

struct parser {
    tok_stream_t* toks;
};

static fort_outcome_t peek_tok(tok_stream_t* toks, tok_t* tok) {
    if (toks == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    if (toks->next == NULL) {
        return FORT_OUTCOME_ERR;
    }

    if (tok != NULL) {
        *tok = *toks->next;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t consume_tok(tok_stream_t* toks, tok_t* tok) {
    fort_outcome_t outcome = peek_tok(toks, tok);
    FORT_OUTCOME_NOK_RET(outcome);

    toks->next = toks->next->next;

    return FORT_OUTCOME_OK;
}

static fort_outcome_t expect(tok_stream_t* toks, tokt_t type, tok_t* tok_out) {
    tok_t tok = {0};
    fort_outcome_t outcome = consume_tok(toks, &tok);
    FORT_OUTCOME_NOK_RET(outcome);

    if (tok.type != type) {
        return FORT_OUTCOME_ERR;
    }

    if (tok_out != NULL) {
        *tok_out = tok;
    }

    return FORT_OUTCOME_OK;
}

static fort_outcome_t parse_int32(const char* p, size_t len, int32_t* num) {
    const int parse_base = 10;
    if (len == 0) {
        return FORT_OUTCOME_FATAL;
    }

    int32_t result = 0;
    for (size_t i = 0; i < len; i++) {
        if (p[i] < '0' || p[i] > '9') {
            return FORT_OUTCOME_FATAL;
        }

        // Check for overflow before multiplication
        if (result > INT32_MAX / parse_base) {
            return FORT_OUTCOME_ERR;
        }
        result *= parse_base;

        int32_t digit = p[i] - '0';

        // Check for overflow before addition
        if (result > (INT32_MAX - digit)) {
            return FORT_OUTCOME_ERR;
        }

        result += digit;
    }

    *num = result;

    return FORT_OUTCOME_OK;
}

static fort_outcome_t parse_constant(tok_stream_t* toks, expr_t* expr) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    tok_t tok = {0};
    outcome = expect(toks, TOKT_CONSTANT, &tok);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = parse_int32(tok.lexeme.p, tok.lexeme.len, &expr->u.constant.val);
    FORT_OUTCOME_NOK_RET(outcome);

    expr->kind = EXPR_CONST;

    return FORT_OUTCOME_OK;
}

static fort_outcome_t parse_expr(tok_stream_t* toks, expr_t* expr);

static fort_outcome_t parse_unary_expr(tok_stream_t* toks, expr_t* expr) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    tok_t tok = {0};
    outcome = consume_tok(toks, &tok);
    FORT_OUTCOME_NOK_RET(outcome);

    if (tok.type != TOKT_TILDE && tok.type != TOKT_MINUS) {
        return FORT_OUTCOME_FATAL;
    }

    expr_t* operand = malloc(sizeof(expr_t));
    if (operand == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = parse_expr(toks, operand);
    FORT_OUTCOME_NOK_RET(outcome);

    expr->kind = EXPR_UNARY;
    expr->u.unary.op = tok.type == TOKT_TILDE ? UNOP_COMPLEMENT : UNOP_NEGATE;
    expr->u.unary.expr = operand;

    return FORT_OUTCOME_OK;
}

static fort_outcome_t parse_expr(tok_stream_t* toks, expr_t* expr) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    tok_t tok = {0};
    outcome = peek_tok(toks, &tok);
    FORT_OUTCOME_NOK_RET(outcome);

    switch (tok.type) {
    case TOKT_CONSTANT:
        outcome = parse_constant(toks, expr);
        break;
    case TOKT_TILDE:
    case TOKT_MINUS: {
        outcome = parse_unary_expr(toks, expr);
        break;
    }
    case TOKT_OPEN_PAREN: {
        outcome = parse_expr(toks, expr);
        FORT_OUTCOME_NOK_RET(outcome);

        outcome = expect(toks, TOKT_CLOSE_PAREN, NULL);
        FORT_OUTCOME_NOK_RET(outcome);
        break;
    }
    default:
        return FORT_OUTCOME_ERR;
        break;
    }

    return outcome;
}

static void expr_fini(expr_t* expr) {
    switch (expr->kind) {
    case EXPR_CONST:
        break;
    case EXPR_UNARY:
        expr_fini(expr->u.unary.expr);
        free(expr->u.unary.expr);
        break;
    }
}

static fort_outcome_t parse_stmt(tok_stream_t* toks, stmt_t* stmt) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    outcome = expect(toks, TOKT_KEYWORD_RETURN, NULL);
    FORT_OUTCOME_NOK_RET(outcome);
    stmt->kind = STMT_RET;

    outcome = parse_expr(toks, &stmt->u.ret.expr);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_SEMICOLON, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static void stmt_fini(stmt_t* stmt) {
    switch (stmt->kind) {
    case STMT_RET:
        expr_fini(&stmt->u.ret.expr);
        break;
    }
}

static fort_outcome_t parse_func(tok_stream_t* toks, func_t* func) {
    fort_outcome_t outcome = FORT_OUTCOME_ERR;

    outcome = expect(toks, TOKT_KEYWORD_I32, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    tok_t ident = {0};
    outcome = expect(toks, TOKT_IDENTIFIER, &ident);
    FORT_OUTCOME_NOK_RET(outcome);
    func->name = ident.lexeme;

    outcome = expect(toks, TOKT_OPEN_PAREN, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_KEYWORD_VOID, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_CLOSE_PAREN, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_OPEN_BRACE, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = parse_stmt(toks, &func->body);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_CLOSE_BRACE, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

static void func_fini(func_t* func) {
    stmt_fini(&func->body);
}

static fort_outcome_t parse_prog(tok_stream_t* toks, prog_t* prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = parse_func(toks, &prog->func);
    FORT_OUTCOME_NOK_RET(outcome);

    outcome = expect(toks, TOKT_EOF, NULL);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

void prog_fini(prog_t* prog) {
    func_fini(&prog->func);
}

parser_t* mkparser(tok_stream_t* toks) {
    parser_t* parser = malloc(sizeof(parser_t));
    parser->toks = toks;

    return parser;
}

void parser_fini(parser_t* parser) {
    free(parser);
}

fort_outcome_t parser_run(parser_t* parser, prog_t* prog) {
    fort_outcome_t outcome = FORT_OUTCOME_FATAL;

    if (parser == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    if (prog == NULL) {
        return FORT_OUTCOME_FATAL;
    }

    outcome = parse_prog(parser->toks, prog);
    FORT_OUTCOME_NOK_RET(outcome);

    return FORT_OUTCOME_OK;
}

// AST printer implementation

static const int AST_INDENT_SIZE = 2;

static void ast_print_indent(int depth) {
    for (int i = 0; i < depth * AST_INDENT_SIZE; ++i) {
        eprint(" ");
    }
}

static const char* unop_to_str(unop_t op) {
    switch (op) {
    case UNOP_COMPLEMENT:
        return "Complement";
    case UNOP_NEGATE:
        return "Negate";
    default:
        return "Unknown";
    }
}

static void ast_print_expr(const expr_t* expr, int depth) {
    ast_print_indent(depth);

    switch (expr->kind) {
    case EXPR_CONST: {
        eprint("Constant(%" PRId32 ")", expr->u.constant.val);
        eprintln("");
        break;
    }
    case EXPR_UNARY: {
        eprintln("UnaryOp(");
        ast_print_indent(depth + 1);
        eprint("op=%s,", unop_to_str(expr->u.unary.op));
        eprintln("");
        ast_print_indent(depth + 1);
        eprint("expr=");
        if (expr->u.unary.expr != NULL) {
            eprintln("");
            ast_print_expr(expr->u.unary.expr, depth + 2);
            ast_print_indent(depth);
        } else {
            eprintln("NULL");
            ast_print_indent(depth);
        }
        eprintln(")");
        break;
    }
    }
}

static void ast_print_stmt(const stmt_t* stmt, int depth) {
    ast_print_indent(depth);

    switch (stmt->kind) {
    case STMT_RET: {
        eprintln("Return(");
        ast_print_expr(&stmt->u.ret.expr, depth + 1);
        ast_print_indent(depth);
        eprintln(")");
        break;
    }
    }
}

static void ast_print_func(const func_t* func, int depth) {
    ast_print_indent(depth);
    eprintln("Function(");

    ast_print_indent(depth + 1);
    eprint("name=\"%.*s\",", (int)func->name.len, func->name.p);
    eprintln("");

    ast_print_indent(depth + 1);
    eprint("body=");
    eprintln("");
    ast_print_stmt(&func->body, depth + 2);

    ast_print_indent(depth);
    eprintln(")");
}

void ast_print(const prog_t* prog) {
    eprintln("Program(");
    ast_print_func(&prog->func, 1);
    eprintln(")");
}
