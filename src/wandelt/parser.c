#include "parser.h"
#include "ast.h"
#include "defines.h"
#include "diagnostics.h"
#include "lexer.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"
#include <assert.h>

static Statement invalid_statement     = {.type = STATEMENT_TYPE_INVALID};
static Declaration invalid_declaration = {.type = DECLARATION_TYPE_INVALID};
static Expression invalid_expression   = {.type = EXPRESSION_TYPE_INVALID};

static ParseRule parse_rules[TOKEN_TYPE_COUNT];

#define new_statement(parser)   (parser)->stmt_allocator->alloc((parser)->stmt_allocator->ctx, sizeof(Statement))
#define new_declaration(parser) (parser)->decl_allocator->alloc((parser)->decl_allocator->ctx, sizeof(Declaration))
#define new_expression(parser)  (parser)->expr_allocator->alloc((parser)->expr_allocator->ctx, sizeof(Expression))

static StringView get_token_lexeme(Parser* parser, Token token)
{
	return file_get_part_of_content(parser->lexer->file_to_lex, token.span.begin, token.span.end - token.span.begin);
}

Parser parser_create(Allocator* stmt_allocator, Allocator* decl_allocator, Allocator* expr_allocator, Lexer* lexer)
{
	TranslationUnit tu;
	tu.file       = lexer->file_to_lex;
	tu.statements = vector_create(stmt_allocator, 8, sizeof(Statement*));

	Parser parser;
	parser.stmt_allocator   = stmt_allocator;
	parser.decl_allocator   = decl_allocator;
	parser.expr_allocator   = expr_allocator;
	parser.lexer            = lexer;
	parser.translation_unit = tu;

	return parser;
}

TranslationUnit parser_parse(Parser* parser)
{
	Statement* last_stmt = nullptr;

	while (parser_peek_token(parser).type != TOKEN_TYPE_EOF)
	{
		Statement* stmt = parser_parse_top_level_statement(parser);
		ASSERT(stmt);

		if (stmt->type == STATEMENT_TYPE_INVALID)
			parser_recover_from_error(parser);

		if (last_stmt != nullptr)
			last_stmt->next = stmt;

		last_stmt = stmt;

		vector_push(parser->translation_unit.statements, stmt);
	}

	return parser->translation_unit;
}

Token parser_peek_token(Parser* parser)
{
	return lexer_peek_token(parser->lexer);
}

void parser_eat_token(Parser* parser)
{
	parser->previous_token = lexer_peek_token(parser->lexer);
	lexer_eat_token(parser->lexer);
}

Statement* parser_parse_top_level_statement(Parser* parser)
{
	static_assert(STATEMENT_TYPE_COUNT == 4,
	              "parser_parse_top_level_statement needs to be updated to handle new statement types");

	Token tok = parser_peek_token(parser);

	switch (tok.type)
	{
	case TOKEN_TYPE_NAMESPACE_KEYWORD:
		return parser_parse_declaration_statement(parser);

	case TOKEN_TYPE_RETURN_KEYWORD:
		return parser_parse_return_statement(parser);

	default:
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex,
		                              "Expected a top-level statement, but found '%.*s'",
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		break;
	};

	return &invalid_statement;
}

void parser_recover_from_error(Parser* parser)
{
	while (parser_peek_token(parser).type != TOKEN_TYPE_EOF)
	{
		switch (parser_peek_token(parser).type)
		{
		// Synchronize and skip past them and resume
		case TOKEN_TYPE_SEMICOLON:
			parser_eat_token(parser);
			return;

		// Synchronize and don't consume them
		case TOKEN_TYPE_FUNCTION_KEYWORD:
		case TOKEN_TYPE_RETURN_KEYWORD:
		case TOKEN_TYPE_NAMESPACE_KEYWORD:
		case TOKEN_TYPE_CLOSE_BRACE:
			return;

		default:
			parser_eat_token(parser);
			break;
		}
	}
}

Statement* parser_parse_statement(Parser* parser)
{
	return &invalid_statement;
}

Statement* parser_parse_declaration_statement(Parser* parser)
{
	Statement* stmt = new_statement(parser);
	stmt->type      = STATEMENT_TYPE_DECLARATION;

	stmt->decl_stmt.declaration = parser_parse_declaration(parser);
	if (stmt->decl_stmt.declaration->type == DECLARATION_TYPE_INVALID)
		return &invalid_statement;

	stmt->span = stmt->decl_stmt.declaration->span;

	return stmt;
}

Statement* parser_parse_expression_statement(Parser* parser)
{
	Statement* stmt = new_statement(parser);
	stmt->type      = STATEMENT_TYPE_EXPRESSION;

	stmt->expr_stmt.expression = parser_parse_expression(parser);
	if (stmt->expr_stmt.expression->type == EXPRESSION_TYPE_INVALID)
		return &invalid_statement;

	const Token semicolonToken = parser_peek_token(parser);
	if (!parser_parse_token(parser, TOKEN_TYPE_SEMICOLON))
		return &invalid_statement;

	stmt->span = span_extend(stmt->expr_stmt.expression->span, semicolonToken.span);

	return stmt;
}

Statement* parser_parse_return_statement(Parser* parser)
{
	Statement* stmt = new_statement(parser);
	stmt->type      = STATEMENT_TYPE_RETURN;

	const Token returnToken = parser_peek_token(parser);
	ASSERT(returnToken.type == TOKEN_TYPE_RETURN_KEYWORD);

	parser_eat_token(parser); // eat 'return' keyword

	stmt->return_stmt.expression = parser_parse_expression(parser);
	if (stmt->return_stmt.expression->type == EXPRESSION_TYPE_INVALID)
		return &invalid_statement;

	const Token semicolonToken = parser_peek_token(parser);
	if (!parser_parse_token(parser, TOKEN_TYPE_SEMICOLON))
		return &invalid_statement;

	stmt->span = span_extend(returnToken.span, semicolonToken.span);

	return stmt;
}

Declaration* parser_parse_declaration(Parser* parser)
{
	static_assert(DECLARATION_TYPE_COUNT == 2,
	              "parser_parse_declaration needs to be updated to handle new declaration types");

	Token tok = parser_peek_token(parser);

	switch (tok.type)
	{
	case TOKEN_TYPE_NAMESPACE_KEYWORD:
		return parser_parse_namespace_declaration(parser);

	default:
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected a declaration, but found '%.*s'",
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		break;
	};

	return &invalid_declaration;
}

Declaration* parser_parse_namespace_declaration(Parser* parser)
{
	const Token namespaceToken = parser_peek_token(parser);
	ASSERT(namespaceToken.type == TOKEN_TYPE_NAMESPACE_KEYWORD);

	parser_eat_token(parser); // eat 'namespace' keyword

	Declaration* decl = new_declaration(parser);
	decl->type        = DECLARATION_TYPE_NAMESPACE;

	if (!parser_parse_identifier(parser, &decl->namespace.name))
		return &invalid_declaration;

	const Token semicolonToken = parser_peek_token(parser);
	if (!parser_parse_token(parser, TOKEN_TYPE_SEMICOLON))
		return &invalid_declaration;

	decl->span = span_extend(namespaceToken.span, semicolonToken.span);

	return decl;
}

Expression* parser_parse_expression(Parser* parser)
{
	return parser_parse_expression_with_precedence(parser, PRECEDENCE_NONE);
}

Expression* parser_parse_expression_with_precedence(Parser* parser, Precedence min_precedence)
{
	Token tok = parser_peek_token(parser);
	if (tok.type == TOKEN_TYPE_INVALID)
		return &invalid_expression;

	PrefixParseFn prefix_rule = parse_rules[tok.type].prefix;
	if (prefix_rule == nullptr)
	{
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected an expression, but found '%.*s'",
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		return &invalid_expression;
	}

	Expression* left = prefix_rule(parser);

	// If the left expression is invalid, we can skip parsing the rest of the expression
	if (parser_peek_token(parser).type == TOKEN_TYPE_INVALID)
		return &invalid_expression;

	while (min_precedence <= parse_rules[parser_peek_token(parser).type].precedence)
	{
		Token infixToken = parser_peek_token(parser);

		InfixParseFn infix_rule = parse_rules[infixToken.type].infix;
		if (infix_rule == nullptr)
			break;

		left = infix_rule(parser, left);
	}

	return left;
}

Expression* parser_parse_constant_expression(Parser* parser)
{
	Token tok = parser_peek_token(parser);
	ASSERT(tok.type == TOKEN_TYPE_INTEGER);

	Expression* expr = new_expression(parser);
	expr->type       = EXPRESSION_TYPE_CONSTANT;

	expr->constant.kind    = CONSTANT_KIND_INTEGER;
	expr->constant.integer = (u64)strtoll(
	    file_get_part_of_content(parser->lexer->file_to_lex, tok.span.begin, tok.span.end - tok.span.begin).data,
	    nullptr, 10);
	expr->span = tok.span;

	parser_eat_token(parser); // eat the integer token

	return expr;
}

Expression* parser_parse_binary_expression(Parser* parser, Expression* left)
{
	Token infixToken = parser_peek_token(parser);

	Expression* expr = new_expression(parser);
	expr->type       = EXPRESSION_TYPE_BINARY;

	expr->binary.operator = token_type_to_binary_operator(infixToken.type);
	expr->binary.left     = left;

	Precedence precedence = parse_rules[infixToken.type].precedence;
	parser_eat_token(parser); // eat the operator token

	// When precedence + 1
	// for 1 + 2 + 3 gets parsed as (1 + 2) + 3
	// When precedence
	// for 1 + 2 + 3 gets parsed as 1 + (2 + 3)
	expr->binary.right = parser_parse_expression_with_precedence(parser, precedence + 1);
	if (expr->binary.right->type == EXPRESSION_TYPE_INVALID)
		return &invalid_expression;

	expr->span = span_extend(left->span, expr->binary.right->span);

	return expr;
}

bool parser_parse_token(Parser* parser, TokenType expected_type)
{
	Token tok = parser_peek_token(parser);

	if (tok.type != expected_type)
	{
		Span after_prev = {.begin = parser->previous_token.span.end, .end = parser->previous_token.span.end};
		diagnostics_verror_along_span(after_prev, parser->lexer->file_to_lex, "Expected '%s', but found '%.*s'",
		                              token_type_to_lexeme_cstr(expected_type),
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		return false;
	}

	parser_eat_token(parser); // eat the expected token

	return true;
}

bool parser_parse_identifier(Parser* parser, StringView* out_identifier)
{
	Token tok = parser_peek_token(parser);

	if (tok.type != TOKEN_TYPE_IDENTIFIER)
	{
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected an identifier, but found '%.*s'",
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		return false;
	}

	StringView view =
	    file_get_part_of_content(parser->lexer->file_to_lex, tok.span.begin, tok.span.end - tok.span.begin);

	*out_identifier = view;

	parser_eat_token(parser); // eat the identifier token

	return true;
}

static ParseRule parse_rules[TOKEN_TYPE_COUNT] = {
    [TOKEN_TYPE_PLUS]    = {nullptr, parser_parse_binary_expression, PRECEDENCE_ADDITIVE},
    [TOKEN_TYPE_MINUS]   = {nullptr, parser_parse_binary_expression, PRECEDENCE_ADDITIVE},
    [TOKEN_TYPE_STAR]    = {nullptr, parser_parse_binary_expression, PRECEDENCE_MULTIPLY},
    [TOKEN_TYPE_SLASH]   = {nullptr, parser_parse_binary_expression, PRECEDENCE_MULTIPLY},
    [TOKEN_TYPE_INTEGER] = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
};

static_assert(TOKEN_TYPE_COUNT == 17, "Update parse_rules when adding new token types");
