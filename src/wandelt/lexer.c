#include "lexer.h"
#include "wandelt/string.h"

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

	const char* saved_current_char  = lexer->current_char;
	u32 saved_lexing_start_offset   = lexer->lexing_start_offset;

	for (i32 i = 0; i < offset; i++)
	{
		lexer_eat_token(lexer);
		result = lexer_peek_token(lexer);
	}

	lexer->current_char        = saved_current_char;
	lexer->lexing_start_offset = saved_lexing_start_offset;

	return result;
}

void lexer_debug_print_token(Lexer* lexer, Token token)
{
	FileLocation loc = file_resolve_location(lexer->file_to_lex, token.span.begin);
	u32 tok_length    = token.span.end - token.span.begin;

	printf("<Parsed token: \"%.*s\" at %.*s:%u:%u />\n",
	       FMT_STR_ARG(file_get_part_of_content(lexer->file_to_lex, token.span.begin, tok_length)),
	       FMT_STR_ARG(lexer->file_to_lex->name), loc.row, loc.col);
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
	return _lexer_lex_token_internal(lexer);
}

Token _lexer_lex_token_internal(Lexer* lexer)
{
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
		Token tok          = _lexer_create_new_token(lexer, TOKEN_TYPE_INVALID);
		FileLocation loc = file_resolve_location(lexer->file_to_lex, tok.span.begin);

		printf("Invalid character '%c' at %.*s:%u:%u\n", c, FMT_STR_ARG(lexer->file_to_lex->name), loc.row, loc.col);

		return tok;
	}

	return token;
}
