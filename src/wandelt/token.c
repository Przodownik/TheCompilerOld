#include "token.h"

const char* token_type_to_cstr(TokenType type)
{
	static_assert(TOKEN_TYPE_COUNT == 52, "Update token_type_to_cstr when adding new token types");

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
	case TOKEN_TYPE_AS_KEYWORD:
		return "TOKEN_TYPE_AS_KEYWORD";
	case TOKEN_TYPE_IF_KEYWORD:
		return "TOKEN_TYPE_IF_KEYWORD";
	case TOKEN_TYPE_ELSE_KEYWORD:
		return "TOKEN_TYPE_ELSE_KEYWORD";
	case TOKEN_TYPE_WHILE_KEYWORD:
		return "TOKEN_TYPE_WHILE_KEYWORD";
	case TOKEN_TYPE_FOR_KEYWORD:
		return "TOKEN_TYPE_FOR_KEYWORD";
	case TOKEN_TYPE_INLINE_KEYWORD:
		return "TOKEN_TYPE_INLINE_KEYWORD";

	case TOKEN_TYPE_BOOL_KEYWORD:
		return "TOKEN_TYPE_BOOL_KEYWORD";
	case TOKEN_TYPE_CHAR_KEYWORD:
		return "TOKEN_TYPE_CHAR_KEYWORD";
	case TOKEN_TYPE_UCHAR_KEYWORD:
		return "TOKEN_TYPE_UCHAR_KEYWORD";
	case TOKEN_TYPE_SHORT_KEYWORD:
		return "TOKEN_TYPE_SHORT_KEYWORD";
	case TOKEN_TYPE_USHORT_KEYWORD:
		return "TOKEN_TYPE_USHORT_KEYWORD";
	case TOKEN_TYPE_INT_KEYWORD:
		return "TOKEN_TYPE_INT_KEYWORD";
	case TOKEN_TYPE_UINT_KEYWORD:
		return "TOKEN_TYPE_UINT_KEYWORD";
	case TOKEN_TYPE_LONG_KEYWORD:
		return "TOKEN_TYPE_LONG_KEYWORD";
	case TOKEN_TYPE_ULONG_KEYWORD:
		return "TOKEN_TYPE_ULONG_KEYWORD";
	case TOKEN_TYPE_FLOAT_KEYWORD:
		return "TOKEN_TYPE_FLOAT_KEYWORD";
	case TOKEN_TYPE_DOUBLE_KEYWORD:
		return "TOKEN_TYPE_DOUBLE_KEYWORD";
	case TOKEN_TYPE_TRUE_KEYWORD:
		return "TOKEN_TYPE_TRUE_KEYWORD";
	case TOKEN_TYPE_FALSE_KEYWORD:
		return "TOKEN_TYPE_FALSE_KEYWORD";

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
	case TOKEN_TYPE_DOT:
		return "TOKEN_TYPE_DOT";
	case TOKEN_TYPE_GREATER:
		return "TOKEN_TYPE_GREATER";
	case TOKEN_TYPE_LESS:
		return "TOKEN_TYPE_LESS";

	case TOKEN_TYPE_GREATER_EQUAL:
		return "TOKEN_TYPE_GREATER_EQUAL";
	case TOKEN_TYPE_LESS_EQUAL:
		return "TOKEN_TYPE_LESS_EQUAL";
	case TOKEN_TYPE_EQUAL_EQUAL:
		return "TOKEN_TYPE_EQUAL_EQUAL";
	case TOKEN_TYPE_BANG_EQUAL:
		return "TOKEN_TYPE_BANG_EQUAL";
	case TOKEN_TYPE_PLUS_EQUAL:
		return "TOKEN_TYPE_PLUS_EQUAL";
	case TOKEN_TYPE_MINUS_EQUAL:
		return "TOKEN_TYPE_MINUS_EQUAL";
	case TOKEN_TYPE_STAR_EQUAL:
		return "TOKEN_TYPE_STAR_EQUAL";
	case TOKEN_TYPE_SLASH_EQUAL:
		return "TOKEN_TYPE_SLASH_EQUAL";
	case TOKEN_TYPE_PLUS_PLUS:
		return "TOKEN_TYPE_PLUS_PLUS";
	case TOKEN_TYPE_MINUS_MINUS:
		return "TOKEN_TYPE_MINUS_MINUS";

	case TOKEN_TYPE_IDENTIFIER:
		return "TOKEN_TYPE_IDENTIFIER";
	case TOKEN_TYPE_INTEGER:
		return "TOKEN_TYPE_INTEGER";
	case TOKEN_TYPE_FLOAT:
		return "TOKEN_TYPE_FLOAT";
	case TOKEN_TYPE_DOUBLE:
		return "TOKEN_TYPE_DOUBLE";

	case TOKEN_TYPE_EOF:
		return "TOKEN_TYPE_EOF";
	default:
		break;
	}

	ASSERT(false, "Unknown token type");
}

const char* token_type_to_lexeme_cstr(TokenType type)
{
	static_assert(TOKEN_TYPE_COUNT == 52, "Update token_type_to_lexeme_cstr when adding new token types");

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
	case TOKEN_TYPE_AS_KEYWORD:
		return "as";
	case TOKEN_TYPE_IF_KEYWORD:
		return "if";
	case TOKEN_TYPE_ELSE_KEYWORD:
		return "else";
	case TOKEN_TYPE_WHILE_KEYWORD:
		return "while";
	case TOKEN_TYPE_FOR_KEYWORD:
		return "for";
	case TOKEN_TYPE_INLINE_KEYWORD:
		return "inline";

	case TOKEN_TYPE_BOOL_KEYWORD:
		return "bool";
	case TOKEN_TYPE_CHAR_KEYWORD:
		return "char";
	case TOKEN_TYPE_UCHAR_KEYWORD:
		return "uchar";
	case TOKEN_TYPE_SHORT_KEYWORD:
		return "short";
	case TOKEN_TYPE_USHORT_KEYWORD:
		return "ushort";
	case TOKEN_TYPE_INT_KEYWORD:
		return "int";
	case TOKEN_TYPE_UINT_KEYWORD:
		return "uint";
	case TOKEN_TYPE_LONG_KEYWORD:
		return "long";
	case TOKEN_TYPE_ULONG_KEYWORD:
		return "ulong";
	case TOKEN_TYPE_FLOAT_KEYWORD:
		return "float";
	case TOKEN_TYPE_DOUBLE_KEYWORD:
		return "double";
	case TOKEN_TYPE_TRUE_KEYWORD:
		return "true";
	case TOKEN_TYPE_FALSE_KEYWORD:
		return "false";

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
	case TOKEN_TYPE_DOT:
		return ".";
	case TOKEN_TYPE_GREATER:
		return ">";
	case TOKEN_TYPE_LESS:
		return "<";

	case TOKEN_TYPE_GREATER_EQUAL:
		return ">=";
	case TOKEN_TYPE_LESS_EQUAL:
		return "<=";
	case TOKEN_TYPE_EQUAL_EQUAL:
		return "==";
	case TOKEN_TYPE_BANG_EQUAL:
		return "!=";
	case TOKEN_TYPE_PLUS_EQUAL:
		return "+=";
	case TOKEN_TYPE_MINUS_EQUAL:
		return "-=";
	case TOKEN_TYPE_STAR_EQUAL:
		return "*=";
	case TOKEN_TYPE_SLASH_EQUAL:
		return "/=";
	case TOKEN_TYPE_PLUS_PLUS:
		return "++";
	case TOKEN_TYPE_MINUS_MINUS:
		return "--";

	case TOKEN_TYPE_IDENTIFIER:
		return "identifier";
	case TOKEN_TYPE_INTEGER:
		return "integer";
	case TOKEN_TYPE_FLOAT:
		return "float";
	case TOKEN_TYPE_DOUBLE:
		return "double";

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
