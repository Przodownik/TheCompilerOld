#include "lexer.h"
#include "diagnostics.h"
#include "wandelt/string.h"
#include <assert.h>
#include <stdio.h>

#define lexer_get_current_char(lexer)  (*(lexer)->current_char)
#define lexer_get_previous_char(lexer) (*((lexer)->current_char - 1))
#define lexer_get_next_char(lexer)     (*((lexer)->current_char + 1))
#define lexer_is_eof(lexer)            (*(lexer)->current_char == '\0')
#define lexer_is_at_newline(lexer)     (*(lexer)->current_char == '\n')

#define is_character_a_digit(c)         (c >= '0' && c <= '9')
#define is_character_an_alphanumeric(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')

static Token invalid_token = {.type = TOKEN_TYPE_INVALID};

Lexer lexer_create(File* file_to_lex)
{
	return (Lexer){
	    .file_to_lex  = file_to_lex,
	    .cached_token = invalid_token,
	    .current_char = file_to_lex->content.data,
	};
}

void lexer_eat_token(Lexer* lexer)
{
	lexer->cached_token = invalid_token;
}

Token lexer_peek_token(Lexer* lexer)
{
	if (lexer->cached_token.type != TOKEN_TYPE_INVALID)
		return lexer->cached_token;

	lexer->cached_token = _lexer_lex_token(lexer);
	return lexer->cached_token;
}

Token lexer_peek_token_at_offset(Lexer* lexer, i32 offset)
{
	if (offset == 0)
		return lexer_peek_token(lexer);

	Token result = invalid_token;

	const char* saved_current_char      = lexer->current_char;
	const u32 saved_lexing_start_offset = lexer->lexing_start_offset;
	const Token saved_cached_token      = lexer->cached_token;

	for (i32 i = 0; i < offset; i++)
	{
		lexer_eat_token(lexer);
		result = lexer_peek_token(lexer);
	}

	lexer->current_char        = saved_current_char;
	lexer->lexing_start_offset = saved_lexing_start_offset;
	lexer->cached_token        = saved_cached_token;

	return result;
}

void lexer_debug_print_token(Lexer* lexer, Token token)
{
	FileLocation loc = file_resolve_location(lexer->file_to_lex, token.span.begin);
	u32 tok_length   = token.span.end - token.span.begin;

	char loc_buf[64];
	snprintf(loc_buf, sizeof(loc_buf), "%.*s:%u:%u", FMT_STR_ARG(lexer->file_to_lex->name), loc.row, loc.col);

	printf("| %-20s | %-20s | %.*s\n", token_type_to_cstr(token.type) + 11, loc_buf,
	       FMT_STR_ARG(file_get_part_of_content(lexer->file_to_lex, token.span.begin, tok_length)));
}

void _lexer_advance(Lexer* lexer)
{
	if (lexer_is_eof(lexer))
		return;

	lexer->current_char++;
}

void _lexer_skip_whitespace(Lexer* lexer)
{
	while (true)
	{
		switch (lexer_get_current_char(lexer))
		{
		case ' ':
		case '\r':
		case '\t':
		case '\n':
			_lexer_advance(lexer);
			break;
		default:
			return;
		};
	}
}

Token _lexer_create_new_token(Lexer* lexer, TokenType type)
{
	Span span = (Span){.begin = lexer->lexing_start_offset,
	                   .end   = (u32)(lexer->current_char - lexer->file_to_lex->content.data)};

	return (Token){.type = type, .span = span};
}

Token _lexer_lex_identifier_or_keyword(Lexer* lexer)
{
	while (is_character_an_alphanumeric(lexer_get_current_char(lexer)) ||
	       is_character_a_digit(lexer_get_current_char(lexer)))
	{
		_lexer_advance(lexer);
	}

	u32 length       = (u32)(lexer->current_char - lexer->file_to_lex->content.data) - lexer->lexing_start_offset;
	StringView ident = {.data = lexer->file_to_lex->content.data + lexer->lexing_start_offset, .len = length};

	ASSERT(length > 0);

	switch (ident.data[0])
	{
	case 'f':
		if (ident.len == 2 && strncmp(ident.data, "fn", 2) == 0)
			return _lexer_create_new_token(lexer, TOKEN_TYPE_FUNCTION_KEYWORD);
		break;
	case 'r':
		if (ident.len == 6 && strncmp(ident.data, "return", 6) == 0)
			return _lexer_create_new_token(lexer, TOKEN_TYPE_RETURN_KEYWORD);
		break;
	case 'n':
		if (ident.len == 9 && strncmp(ident.data, "namespace", 9) == 0)
			return _lexer_create_new_token(lexer, TOKEN_TYPE_NAMESPACE_KEYWORD);
		break;
	case 'i':
		if (ident.len == 3 && strncmp(ident.data, "int", 3) == 0)
			return _lexer_create_new_token(lexer, TOKEN_TYPE_INT_KEYWORD);
		break;
	case 'v':
		if (ident.len == 3 && strncmp(ident.data, "var", 3) == 0)
			return _lexer_create_new_token(lexer, TOKEN_TYPE_VAR_KEYWORD);
	}

	return _lexer_create_new_token(lexer, TOKEN_TYPE_IDENTIFIER);
}

Token _lexer_lex_digit(Lexer* lexer)
{
	while (is_character_a_digit(lexer_get_current_char(lexer)))
	{
		_lexer_advance(lexer);
	}

	return _lexer_create_new_token(lexer, TOKEN_TYPE_INTEGER);
}

Token _lexer_lex_token(Lexer* lexer)
{
	Token token = _lexer_lex_token_internal(lexer);
	if (token.type != TOKEN_TYPE_INVALID)
		return token;

	return _lexer_lex_token(lexer);
}

Token _lexer_lex_token_internal(Lexer* lexer)
{
	static_assert(TOKEN_TYPE_COUNT == 19, "Update _lexer_lex_token_internal when adding new token types");

	_lexer_skip_whitespace(lexer);

	lexer->lexing_start_offset = (u32)(lexer->current_char - lexer->file_to_lex->content.data);

	if (lexer_is_eof(lexer))
		return _lexer_create_new_token(lexer, TOKEN_TYPE_EOF);

	Token token = invalid_token;

	const char c = lexer_get_current_char(lexer);
	_lexer_advance(lexer);

	switch (c)
	{
	case '(':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_OPEN_PAREN);
		break;
	case ')':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_CLOSE_PAREN);
		break;
	case '{':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_OPEN_BRACE);
		break;
	case '}':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_CLOSE_BRACE);
		break;
	case ';':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_SEMICOLON);
		break;
	case '+':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_PLUS);
		break;
	case '-':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_MINUS);
		break;
	case '*':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_STAR);
		break;
	case '/':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_SLASH);
		break;
	case '=':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_EQUALS);
		break;

	default:
		if (is_character_a_digit(c))
		{
			token = _lexer_lex_digit(lexer);
			break;
		}
		if (is_character_an_alphanumeric(c))
		{
			token = _lexer_lex_identifier_or_keyword(lexer);
			break;
		}
	};

	if (token.type == TOKEN_TYPE_INVALID)
	{
		Token tok = _lexer_create_new_token(lexer, TOKEN_TYPE_INVALID);
		diagnostics_verror_along_span(
		    tok.span, lexer->file_to_lex,
		    "Unexpected character '%c', this character is not recognized as valid in the language.", c);

		return tok;
	}

	return token;
}
