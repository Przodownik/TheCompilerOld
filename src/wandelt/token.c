#include "token.h"

const char* token_type_to_cstr(TokenType type)
{
	static_assert(TOKEN_TYPE_COUNT == 19, "Update token_type_to_cstr when adding new token types");

	switch (type)
	{
	case TOKEN_TYPE_INVALID:
		return "TOKEN_TYPE_INVALID";

	case TOKEN_TYPE_FUNCTION_KEYWORD:
		return "TOKEN_TYPE_FUNCTION_KEYWORD";
	case TOKEN_TYPE_RETURN_KEYWORD:
		return "TOKEN_TYPE_RETURN_KEYWORD";
	case TOKEN_TYPE_NAMESPACE_KEYWORD:
		return "TOKEN_TYPE_NAMESPACE_KEYWORD";
	case TOKEN_TYPE_VAR_KEYWORD:
		return "TOKEN_TYPE_VAR_KEYWORD";

	case TOKEN_TYPE_INT_KEYWORD:
		return "TOKEN_TYPE_INT_KEYWORD";

	case TOKEN_TYPE_OPEN_PAREN:
		return "TOKEN_TYPE_OPEN_PAREN";
	case TOKEN_TYPE_CLOSE_PAREN:
		return "TOKEN_TYPE_CLOSE_PAREN";
	case TOKEN_TYPE_OPEN_BRACE:
		return "TOKEN_TYPE_OPEN_BRACE";
	case TOKEN_TYPE_CLOSE_BRACE:
		return "TOKEN_TYPE_CLOSE_BRACE";
	case TOKEN_TYPE_SEMICOLON:
		return "TOKEN_TYPE_SEMICOLON";
	case TOKEN_TYPE_PLUS:
		return "TOKEN_TYPE_PLUS";
	case TOKEN_TYPE_MINUS:
		return "TOKEN_TYPE_MINUS";
	case TOKEN_TYPE_STAR:
		return "TOKEN_TYPE_STAR";
	case TOKEN_TYPE_SLASH:
		return "TOKEN_TYPE_SLASH";
	case TOKEN_TYPE_EQUALS:
		return "TOKEN_TYPE_EQUALS";

	case TOKEN_TYPE_IDENTIFIER:
		return "TOKEN_TYPE_IDENTIFIER";
	case TOKEN_TYPE_INTEGER:
		return "TOKEN_TYPE_INTEGER";
	case TOKEN_TYPE_EOF:
		return "TOKEN_TYPE_EOF";
	default:
		break;
	}

	ASSERT(false, "Unknown token type");
}

const char* token_type_to_lexeme_cstr(TokenType type)
{
	static_assert(TOKEN_TYPE_COUNT == 19, "Update token_type_to_lexeme_cstr when adding new token types");

	switch (type)
	{
	case TOKEN_TYPE_INVALID:
		return "invalid";

	case TOKEN_TYPE_FUNCTION_KEYWORD:
		return "fn";
	case TOKEN_TYPE_RETURN_KEYWORD:
		return "return";
	case TOKEN_TYPE_NAMESPACE_KEYWORD:
		return "namespace";
	case TOKEN_TYPE_VAR_KEYWORD:
		return "var";

	case TOKEN_TYPE_INT_KEYWORD:
		return "int";

	case TOKEN_TYPE_OPEN_PAREN:
		return "(";
	case TOKEN_TYPE_CLOSE_PAREN:
		return ")";
	case TOKEN_TYPE_OPEN_BRACE:
		return "{";
	case TOKEN_TYPE_CLOSE_BRACE:
		return "}";
	case TOKEN_TYPE_SEMICOLON:
		return ";";
	case TOKEN_TYPE_PLUS:
		return "+";
	case TOKEN_TYPE_MINUS:
		return "-";
	case TOKEN_TYPE_STAR:
		return "*";
	case TOKEN_TYPE_SLASH:
		return "/";
	case TOKEN_TYPE_EQUALS:
		return "=";

	case TOKEN_TYPE_IDENTIFIER:
		return "identifier";
	case TOKEN_TYPE_INTEGER:
		return "integer";
	case TOKEN_TYPE_EOF:
		return "EOF";
	default:
		break;
	}

	ASSERT(false, "Unknown token type");
}

Span span_extend(Span a, Span b)
{
	return (Span){
	    .begin = a.begin,
	    .end   = b.end,
	};
}
