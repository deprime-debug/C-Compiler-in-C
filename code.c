#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// --- 1. LEXER ---

typedef enum
{
    TOKEN_INT,
    TOKEN_IDENT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_RETURN,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    int int_val;
    char name[64];
} Token;

typedef struct
{
    const char *src;
    size_t pos;
} Lexer;

Token lexer_next(Lexer *l)
{
    while (l->src[l->pos] != '\0')
    {
        char c = l->src[l->pos];

        if (isspace((unsigned char)c))
        {
            l->pos++;
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_')
        {
            size_t start = l->pos;
            while (isalnum((unsigned char)l->src[l->pos]) || l->src[l->pos] == '_')
            {
                l->pos++;
            }
            size_t len = l->pos - start;
            char buf[64] = {0};
            if (len >= sizeof(buf))
                len = sizeof(buf) - 1;
            strncpy(buf, &l->src[start], len);

            if (strcmp(buf, "int") == 0)
                return (Token){TOKEN_INT, 0, ""};
            if (strcmp(buf, "return") == 0)
                return (Token){TOKEN_RETURN, 0, ""};

            Token t = {TOKEN_IDENT, 0, ""};
            strncpy(t.name, buf, sizeof(t.name) - 1);
            return t;
        }

        if (isdigit((unsigned char)c))
        {
            int val = 0;
            while (isdigit((unsigned char)l->src[l->pos]))
            {
                val = val * 10 + (l->src[l->pos] - '0');
                l->pos++;
            }
            return (Token){TOKEN_NUMBER, val, ""};
        }

        l->pos++;
        switch (c)
        {
        case '(':
            return (Token){TOKEN_LPAREN, 0, ""};
        case ')':
            return (Token){TOKEN_RPAREN, 0, ""};
        case '{':
            return (Token){TOKEN_LBRACE, 0, ""};
        case '}':
            return (Token){TOKEN_RBRACE, 0, ""};
        case '+':
            return (Token){TOKEN_PLUS, 0, ""};
        case '-':
            return (Token){TOKEN_MINUS, 0, ""};
        case '*':
            return (Token){TOKEN_MULTIPLY, 0, ""};
        case ';':
            return (Token){TOKEN_SEMICOLON, 0, ""};
        default:
            fprintf(stderr, "Unexpected character: '%c'\n", c);
            exit(1);
        }
    }
    return (Token){TOKEN_EOF, 0, ""};
}

// --- 2. AST & PARSER ---

typedef enum
{
    AST_PROGRAM,
    AST_FUNCTION,
    AST_RETURN,
    AST_INT_LITERAL,
    AST_BIN_OP
} ASTType;

typedef struct ASTNode
{
    ASTType type;
    union
    {
        struct
        {
            char name[64];
            struct ASTNode *body;
        } function;
        struct
        {
            struct ASTNode *expr;
        } return_stmt;
        int int_val;
        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
            int op;
        } bin_op;
        struct
        {
            struct ASTNode *func;
        } program;
    };
} ASTNode;

typedef struct
{
    Lexer *lexer;
    Token current_token;
} Parser;

void parser_advance(Parser *p)
{
    p->current_token = lexer_next(p->lexer);
}

void parser_expect(Parser *p, TokenType type)
{
    if (p->current_token.type != type)
    {
        fprintf(stderr, "Syntax Error: Expected token type %d, got %d\n", type, p->current_token.type);
        exit(1);
    }
    parser_advance(p);
}

ASTNode *parse_primary(Parser *p)
{
    ASTNode *node = NULL;
    if (p->current_token.type == TOKEN_NUMBER)
    {
        node = malloc(sizeof(ASTNode));
        node->type = AST_INT_LITERAL;
        node->int_val = p->current_token.int_val;
        parser_advance(p);
    }
    else if (p->current_token.type == TOKEN_LPAREN)
    {
        parser_advance(p);
        extern ASTNode *parse_expression(Parser * p);
        node = parse_expression(p);
        parser_expect(p, TOKEN_RPAREN);
    }
    else
    {
        fprintf(stderr, "Expected integer literal or '('\n");
        exit(1);
    }
    return node;
}

ASTNode *parse_expression(Parser *p)
{
    ASTNode *left = parse_primary(p);

    while (p->current_token.type == TOKEN_PLUS ||
           p->current_token.type == TOKEN_MINUS ||
           p->current_token.type == TOKEN_MULTIPLY)
    {
        int op = 0;
        if (p->current_token.type == TOKEN_PLUS)
            op = 0;
        else if (p->current_token.type == TOKEN_MINUS)
            op = 1;
        else if (p->current_token.type == TOKEN_MULTIPLY)
            op = 2;

        parser_advance(p);
        ASTNode *right = parse_primary(p);

        ASTNode *bin = malloc(sizeof(ASTNode));
        bin->type = AST_BIN_OP;
        bin->bin_op.left = left;
        bin->bin_op.right = right;
        bin->bin_op.op = op;
        left = bin;
    }

    return left;
}

ASTNode *parse_statement(Parser *p)
{
    parser_expect(p, TOKEN_RETURN);
    ASTNode *expr = parse_expression(p);
    parser_expect(p, TOKEN_SEMICOLON);

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN;
    node->return_stmt.expr = expr;
    return node;
}

ASTNode *parse_function(Parser *p)
{
    parser_expect(p, TOKEN_INT);
    if (p->current_token.type != TOKEN_IDENT)
    {
        fprintf(stderr, "Expected function name\n");
        exit(1);
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION;
    strncpy(node->function.name, p->current_token.name, sizeof(node->function.name) - 1);
    parser_advance(p);

    parser_expect(p, TOKEN_LPAREN);
    parser_expect(p, TOKEN_RPAREN);
    parser_expect(p, TOKEN_LBRACE);

    node->function.body = parse_statement(p);

    parser_expect(p, TOKEN_RBRACE);
    return node;
}

ASTNode *parse_program(Parser *p)
{
    parser_advance(p);
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->program.func = parse_function(p);
    return node;
}

// --- 3. CODE GENERATOR ---

void codegen(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_PROGRAM:
        codegen(node->program.func);
        break;

    case AST_FUNCTION:
        printf("  .globl %s\n", node->function.name);
        printf("%s:\n", node->function.name);
        printf("  pushq %%rbp\n");
        printf("  movq %%rsp, %%rbp\n");

        codegen(node->function.body);

        printf("  movq %%rbp, %%rsp\n");
        printf("  popq %%rbp\n");
        printf("  ret\n");
        break;

    case AST_RETURN:
        codegen(node->return_stmt.expr);
        break;

    case AST_INT_LITERAL:
        printf("  movq $%d, %%rax\n", node->int_val);
        break;

    case AST_BIN_OP:
        codegen(node->bin_op.left);
        printf("  pushq %%rax\n"); // Preserve left result on stack
        codegen(node->bin_op.right);
        printf("  movq %%rax, %%rcx\n"); // Right side -> %rcx
        printf("  popq %%rax\n");        // Left side -> %rax

        if (node->bin_op.op == 0)
        {
            printf("  addq %%rcx, %%rax\n");
        }
        else if (node->bin_op.op == 1)
        {
            printf("  subq %%rcx, %%rax\n");
        }
        else if (node->bin_op.op == 2)
        {
            printf("  imulq %%rcx, %%rax\n");
        }
        break;
    }
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;
    switch (node->type)
    {
    case AST_PROGRAM:
        free_ast(node->program.func);
        break;
    case AST_FUNCTION:
        free_ast(node->function.body);
        break;
    case AST_RETURN:
        free_ast(node->return_stmt.expr);
        break;
    case AST_BIN_OP:
        free_ast(node->bin_op.left);
        free_ast(node->bin_op.right);
        break;
    case AST_INT_LITERAL:
        break;
    }
    free(node);
}

// --- 4. MAIN ---

int main(void)
{
    const char *source = "int main() { return 42 + 8 * 2; }";

    Lexer lexer = {.src = source, .pos = 0};
    Parser parser = {.lexer = &lexer};

    ASTNode *ast = parse_program(&parser);
    codegen(ast);
    free_ast(ast);

    return 0;
}
