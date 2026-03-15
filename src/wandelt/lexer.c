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
	    .file_to_lex     = file_to_lex,
	    .cached_token    = invalid_token,
	    .current_char    = file_to_lex->content.data,
	    .lexing_start    = file_to_lex->content.data,
	    .line_start_char = file_to_lex->content.data,
	    .current_row     = 1u,
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

	const char* saved_current_char    = lexer->current_char;
	const char* saved_lexing_start    = lexer->lexing_start;
	const char* saved_line_start_char = lexer->line_start_char;
	u32 saved_current_row             = lexer->current_row;

	for (i32 i = 0; i < offset; i++)
	{
		lexer_eat_token(lexer);
		result = lexer_peek_token(lexer);
	}

	lexer->current_char    = saved_current_char;
	lexer->lexing_start    = saved_lexing_start;
	lexer->line_start_char = saved_line_start_char;
	lexer->current_row     = saved_current_row;

	return result;
}

void _lexer_advance(Lexer* lexer)
{
	if (lexer_is_eof(lexer))
		return;

	if (lexer_is_at_newline(lexer))
	{
		lexer->line_start_char = lexer->current_char + 1;
		lexer->current_row++;
	}

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

Token _lexer_create_new_token(Lexer* lexer, TokenType type, StringView lexeme)
{
	SourceLocation location = (SourceLocation){.start_row = lexer->lexing_start_row,
	                                           .start_col = (u32)(lexer->lexing_start - lexer->line_start_char) + 1,
	                                           .end_row   = lexer->current_row,
	                                           .end_col   = (u32)(lexer->current_char - lexer->line_start_char) + 1,
	                                           .filename  = lexer->file_to_lex->name};

	return (Token){.type = type, .lexeme = lexeme, .source_location = location};
}

Token _lexer_lex_identifier_or_keyword(Lexer* lexer)
{
	while (is_character_an_alphanumeric(lexer_get_current_char(lexer)) ||
	       is_character_a_digit(lexer_get_current_char(lexer)))
	{
		_lexer_advance(lexer);
	}

	u32 lexeme_length = (u32)(lexer->current_char - lexer->lexing_start);
	StringView lexeme = string_view_from_cstr_part(lexer->lexing_start, lexeme_length);

	return _lexer_create_new_token(lexer, TOKEN_TYPE_IDENTIFIER, lexeme);
}

Token _lexer_lex_digit(Lexer* lexer)
{
	while (is_character_a_digit(lexer_get_current_char(lexer)))
	{
		_lexer_advance(lexer);
	}

	u32 lexeme_length = (u32)(lexer->current_char - lexer->lexing_start);
	StringView lexeme = string_view_from_cstr_part(lexer->lexing_start, lexeme_length);

	return _lexer_create_new_token(lexer, TOKEN_TYPE_INTEGER, lexeme);
}

Token _lexer_lex_token(Lexer* lexer)
{
	Token token = _lexer_lex_token_internal(lexer);
	if (token.type != TOKEN_TYPE_INVALID)
		return token;

	lexer_eat_token(lexer);

	while (token.type != TOKEN_TYPE_EOF)
	{
		token = lexer_peek_token(lexer);
		lexer_eat_token(lexer);
	}

	exit(-1);
}

Token _lexer_lex_token_internal(Lexer* lexer)
{
	_lexer_skip_whitespace(lexer);

	lexer->lexing_start     = lexer->current_char;
	lexer->lexing_start_row = lexer->current_row;

	if (lexer_is_eof(lexer))
		return _lexer_create_new_token(lexer, TOKEN_TYPE_EOF, string_view_from_cstr("EOF"));

	Token token = invalid_token;

	const char c = lexer_get_current_char(lexer);
	_lexer_advance(lexer);

	switch (c)
	{
	case '(':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_OPEN_PAREN, string_view_from_cstr("("));
		break;
	case ')':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_CLOSE_PAREN, string_view_from_cstr(")"));
		break;
	case '{':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_OPEN_BRACE, string_view_from_cstr("{"));
		break;
	case '}':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_CLOSE_BRACE, string_view_from_cstr("}"));
		break;
	case ';':
		token = _lexer_create_new_token(lexer, TOKEN_TYPE_SEMICOLON, string_view_from_cstr(";"));
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
		Token tok =
		    _lexer_create_new_token(lexer, TOKEN_TYPE_INVALID, string_view_from_cstr_part(lexer->current_char, 1));

		printf("Invalid character '%c' at %.*s:%u:%u\n", c, FMT_STR_ARG(lexer->file_to_lex->name),
		       tok.source_location.start_row, tok.source_location.start_col);

		return tok;
	}

	return token;
}
