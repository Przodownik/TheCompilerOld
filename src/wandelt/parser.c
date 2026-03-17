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

#define new_statement(parser)   (parser)->stmt_allocator->alloc((parser)->stmt_allocator->ctx, sizeof(Statement))
#define new_declaration(parser) (parser)->decl_allocator->alloc((parser)->decl_allocator->ctx, sizeof(Declaration))
#define new_expression(parser)  (parser)->expr_allocator->alloc((parser)->expr_allocator->ctx, sizeof(Expression))

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
		                              "Expected an top-level statement, but found '%s'", token_type_to_cstr(tok.type));
		break;
	};

	return &invalid_statement;
}

void parser_recover_from_error(Parser* parser)
{
	diagnostics_vnote_along_span(parser_peek_token(parser).span, parser->lexer->file_to_lex,
	                             "Skipping token '%s' to recover from error",
	                             token_type_to_cstr(parser_peek_token(parser).type));
	parser_eat_token(parser);

	while (parser_peek_token(parser).type != TOKEN_TYPE_EOF)
	{
		switch (parser_peek_token(parser).type)
		{
			// todo functions etc..
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
	Statement* stmt             = new_statement(parser);
	stmt->type                  = STATEMENT_TYPE_DECLARATION;
	stmt->decl_stmt.declaration = parser_parse_declaration(parser);

	if (stmt->decl_stmt.declaration->type == DECLARATION_TYPE_INVALID)
		return &invalid_statement;

	return stmt;
}

Statement* parser_parse_expression_statement(Parser* parser)
{
	Statement* stmt            = new_statement(parser);
	stmt->type                 = STATEMENT_TYPE_EXPRESSION;
	stmt->expr_stmt.expression = parser_parse_expression(parser);

	if (stmt->expr_stmt.expression->type == EXPRESSION_TYPE_INVALID)
		return &invalid_statement;

	return stmt;
}

Statement* parser_parse_return_statement(Parser* parser)
{
	return &invalid_statement;
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
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected a declaration, but found '%s'",
		                              token_type_to_cstr(tok.type));
		break;
	};

	return &invalid_declaration;
}

Declaration* parser_parse_namespace_declaration(Parser* parser)
{
	Token tok = parser_peek_token(parser);
	ASSERT(tok.type == TOKEN_TYPE_NAMESPACE_KEYWORD);

	parser_eat_token(parser); // eat 'namespace' keyword

	Declaration* decl = new_declaration(parser);
	decl->type        = DECLARATION_TYPE_NAMESPACE;

	if (!parser_parse_identifier(parser, &decl->namespace.name))
		return &invalid_declaration;

	if (!parser_parse_token(parser, TOKEN_TYPE_SEMICOLON))
		return &invalid_declaration;

	return decl;
}

Expression* parser_parse_expression(Parser* parser)
{
	return &invalid_expression;
}

bool parser_parse_token(Parser* parser, TokenType expected_type)
{
	Token tok = parser_peek_token(parser);

	if (tok.type != expected_type)
	{
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected token '%s', but found '%s'",
		                              token_type_to_cstr(expected_type), token_type_to_cstr(tok.type));
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
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected an identifier, but found '%s'",
		                              token_type_to_cstr(tok.type));
		return false;
	}

	StringView view =
	    file_get_part_of_content(parser->lexer->file_to_lex, tok.span.begin, tok.span.end - tok.span.begin);

	*out_identifier = view;

	parser_eat_token(parser); // eat the identifier token

	return true;
}
