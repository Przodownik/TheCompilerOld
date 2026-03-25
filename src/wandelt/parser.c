#include "parser.h"

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/diagnostics.h"
#include "wandelt/lexer.h"
#include "wandelt/string.h"
#include "wandelt/token.h"
#include "wandelt/vector.h"

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

	case TOKEN_TYPE_VAR_KEYWORD:
		return parser_parse_declaration_statement(parser);

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
			return;

		// Synchronize and skip past closing braces
		case TOKEN_TYPE_CLOSE_BRACE:
			parser_eat_token(parser);
			return;

		default:
			parser_eat_token(parser);
			break;
		}
	}
}

Statement* parser_parse_statement(Parser* parser)
{
	(void)parser;
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
	static_assert(DECLARATION_TYPE_COUNT == 3,
	              "parser_parse_declaration needs to be updated to handle new declaration types");

	Token tok = parser_peek_token(parser);

	switch (tok.type)
	{
	case TOKEN_TYPE_NAMESPACE_KEYWORD:
		return parser_parse_namespace_declaration(parser);

	case TOKEN_TYPE_VAR_KEYWORD:
		return parser_parse_variable_declaration(parser);

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

Declaration* parser_parse_variable_declaration(Parser* parser)
{
	const Token varToken = parser_peek_token(parser);
	ASSERT(varToken.type == TOKEN_TYPE_VAR_KEYWORD);

	parser_eat_token(parser); // eat 'var' keyword

	Declaration* decl = new_declaration(parser);
	decl->type        = DECLARATION_TYPE_VARIABLE;

	if (!parser_parse_type(parser, &decl->variable.type))
		return &invalid_declaration;

	if (!parser_parse_identifier(parser, &decl->variable.name))
		return &invalid_declaration;

	if (!parser_parse_token(parser, TOKEN_TYPE_EQUALS))
		return &invalid_declaration;

	decl->variable.initializer = parser_parse_expression(parser);
	if (decl->variable.initializer->type == EXPRESSION_TYPE_INVALID)
		return &invalid_declaration;

	const Token semicolonToken = parser_peek_token(parser);
	if (!parser_parse_token(parser, TOKEN_TYPE_SEMICOLON))
		return &invalid_declaration;

	decl->span = span_extend(varToken.span, semicolonToken.span);

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
	ASSERT(tok.type == TOKEN_TYPE_INTEGER || tok.type == TOKEN_TYPE_FLOAT || tok.type == TOKEN_TYPE_DOUBLE ||
	       tok.type == TOKEN_TYPE_TRUE_KEYWORD || tok.type == TOKEN_TYPE_FALSE_KEYWORD);

	switch (tok.type)
	{
	case TOKEN_TYPE_INTEGER:
		return parser_parse_integer_constant_expression(parser);

	case TOKEN_TYPE_FLOAT:
		return parser_parse_float_constant_expression(parser);

	case TOKEN_TYPE_DOUBLE:
		return parser_parse_double_constant_expression(parser);

	case TOKEN_TYPE_TRUE_KEYWORD:
	case TOKEN_TYPE_FALSE_KEYWORD:
		return parser_parse_boolean_constant_expression(parser);

	default:
		ASSERT(false, "unhandled constant token type");
		return &invalid_expression;
	};
}

Expression* parser_parse_integer_constant_expression(Parser* parser)
{
	Expression* expr    = new_expression(parser);
	expr->type          = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind = CONSTANT_KIND_INTEGER;
	expr->constant.integer_value =
	    (u64)strtoll(file_get_part_of_content(parser->lexer->file_to_lex, parser_peek_token(parser).span.begin,
	                                          parser_peek_token(parser).span.end - parser_peek_token(parser).span.begin)
	                     .data,
	                 nullptr, 10);
	expr->span = parser_peek_token(parser).span;

	parser_eat_token(parser); // eat the integer token

	return expr;
}

Expression* parser_parse_float_constant_expression(Parser* parser)
{
	Expression* expr    = new_expression(parser);
	expr->type          = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind = CONSTANT_KIND_FLOAT;
	expr->constant.float_value =
	    strtof(file_get_part_of_content(parser->lexer->file_to_lex, parser_peek_token(parser).span.begin,
	                                    parser_peek_token(parser).span.end - parser_peek_token(parser).span.begin)
	               .data,
	           nullptr);
	expr->span = parser_peek_token(parser).span;

	parser_eat_token(parser); // eat the float token

	return expr;
}

Expression* parser_parse_double_constant_expression(Parser* parser)
{
	Expression* expr    = new_expression(parser);
	expr->type          = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind = CONSTANT_KIND_DOUBLE;
	expr->constant.double_value =
	    strtod(file_get_part_of_content(parser->lexer->file_to_lex, parser_peek_token(parser).span.begin,
	                                    parser_peek_token(parser).span.end - parser_peek_token(parser).span.begin)
	               .data,
	           nullptr);
	expr->span = parser_peek_token(parser).span;

	parser_eat_token(parser); // eat the double token

	return expr;
}

Expression* parser_parse_boolean_constant_expression(Parser* parser)
{
	Token tok = parser_peek_token(parser);
	ASSERT(tok.type == TOKEN_TYPE_TRUE_KEYWORD || tok.type == TOKEN_TYPE_FALSE_KEYWORD);

	Expression* expr             = new_expression(parser);
	expr->type                   = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind          = CONSTANT_KIND_BOOLEAN;
	expr->constant.boolean_value = (tok.type == TOKEN_TYPE_TRUE_KEYWORD);
	expr->span                   = tok.span;

	parser_eat_token(parser); // eat the boolean token

	return expr;
}

Expression* parser_parse_unary_expression(Parser* parser)
{
	Token opToken = parser_peek_token(parser);
	ASSERT(opToken.type == TOKEN_TYPE_MINUS);

	parser_eat_token(parser); // eat PREFIX unary operator token

	Expression* expr     = new_expression(parser);
	expr->type           = EXPRESSION_TYPE_UNARY;
	expr->unary.operator = UNARY_OPERATOR_NEGATE; // todo other unary operators
	expr->unary.operand  = parser_parse_expression_with_precedence(parser, PRECEDENCE_UNARY);
	if (expr->unary.operand->type == EXPRESSION_TYPE_INVALID)
		return &invalid_expression;

	expr->span = span_extend(opToken.span, expr->unary.operand->span);

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

Expression* parser_parse_group_expression(Parser* parser)
{
	const Token openParenToken = parser_peek_token(parser);
	ASSERT(openParenToken.type == TOKEN_TYPE_OPEN_PAREN);

	parser_eat_token(parser); // eat '('

	Expression* expr = new_expression(parser);
	expr->type       = EXPRESSION_TYPE_GROUP;

	expr->group.inner = parser_parse_expression(parser);
	if (expr->group.inner->type == EXPRESSION_TYPE_INVALID)
		return &invalid_expression;

	const Token closeParenToken = parser_peek_token(parser);
	if (!parser_parse_token(parser, TOKEN_TYPE_CLOSE_PAREN))
		return &invalid_expression;

	expr->span = span_extend(openParenToken.span, closeParenToken.span);

	return expr;
}

Expression* parser_parse_identifier_expression(Parser* parser)
{
	Token tok = parser_peek_token(parser);
	ASSERT(tok.type == TOKEN_TYPE_IDENTIFIER);

	Expression* expr = new_expression(parser);
	expr->type       = EXPRESSION_TYPE_IDENTIFIER;

	expr->identifier.name =
	    file_get_part_of_content(parser->lexer->file_to_lex, tok.span.begin, tok.span.end - tok.span.begin);
	expr->span = tok.span;

	parser_eat_token(parser); // eat the identifier token

	return expr;
}

Expression* parser_parse_cast_expression(Parser* parser, Expression* left)
{
	const Token asToken = parser_peek_token(parser);
	ASSERT(asToken.type == TOKEN_TYPE_AS_KEYWORD);

	parser_eat_token(parser); // eat 'as' keyword

	Expression* expr = new_expression(parser);
	expr->type       = EXPRESSION_TYPE_CAST;

	if (!parser_parse_type(parser, &expr->cast.target_type))
		return &invalid_expression;

	expr->cast.expression = left;
	expr->span            = span_extend(left->span, parser_peek_token(parser).span);

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

bool parser_parse_type(Parser* parser, Type** out_type)
{
	static_assert(TYPE_KIND_COUNT == 12, "parser_parse_type needs to be updated to handle new types");

	Token tok = parser_peek_token(parser);

	switch (tok.type)
	{
	case TOKEN_TYPE_BOOL_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_BOOL);
		break;
	case TOKEN_TYPE_CHAR_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_CHAR);
		break;
	case TOKEN_TYPE_UCHAR_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_UCHAR);
		break;
	case TOKEN_TYPE_SHORT_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_SHORT);
		break;
	case TOKEN_TYPE_USHORT_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_USHORT);
		break;
	case TOKEN_TYPE_INT_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_INT);
		break;
	case TOKEN_TYPE_UINT_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_UINT);
		break;
	case TOKEN_TYPE_LONG_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_LONG);
		break;
	case TOKEN_TYPE_ULONG_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_ULONG);
		break;
	case TOKEN_TYPE_FLOAT_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_FLOAT);
		break;
	case TOKEN_TYPE_DOUBLE_KEYWORD:
		*out_type = type_get_builtin(TYPE_KIND_DOUBLE);
		break;
	default:
		diagnostics_verror_along_span(tok.span, parser->lexer->file_to_lex, "Expected a type, but found '%.*s'",
		                              FMT_STR_ARG(get_token_lexeme(parser, tok)));
		return false;
	}

	parser_eat_token(parser); // eat the type token

	return true;
}

static ParseRule parse_rules[TOKEN_TYPE_COUNT] = {
    [TOKEN_TYPE_AS_KEYWORD]    = {nullptr, parser_parse_cast_expression, PRECEDENCE_CAST},
    [TOKEN_TYPE_OPEN_PAREN]    = {parser_parse_group_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_PLUS]          = {nullptr, parser_parse_binary_expression, PRECEDENCE_ADDITIVE},
    [TOKEN_TYPE_MINUS]         = {parser_parse_unary_expression, parser_parse_binary_expression, PRECEDENCE_ADDITIVE},
    [TOKEN_TYPE_STAR]          = {nullptr, parser_parse_binary_expression, PRECEDENCE_MULTIPLY},
    [TOKEN_TYPE_SLASH]         = {nullptr, parser_parse_binary_expression, PRECEDENCE_MULTIPLY},
    [TOKEN_TYPE_GREATER]       = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_LESS]          = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_GREATER_EQUAL] = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_LESS_EQUAL]    = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_EQUAL_EQUAL]   = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_BANG_EQUAL]    = {nullptr, parser_parse_binary_expression, PRECEDENCE_COMPARISON},
    [TOKEN_TYPE_INTEGER]       = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_FLOAT]         = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_DOUBLE]        = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_TRUE_KEYWORD]  = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_FALSE_KEYWORD] = {parser_parse_constant_expression, nullptr, PRECEDENCE_NONE},
    [TOKEN_TYPE_IDENTIFIER]    = {parser_parse_identifier_expression, nullptr, PRECEDENCE_NONE},
};

static_assert(TOKEN_TYPE_COUNT == 41, "Update parse_rules when adding new token types");
