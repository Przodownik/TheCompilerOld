/**
 * @file lexer.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/file.h"

typedef enum TokenType
{
	TOKEN_TYPE_INVALID = 0,

	// Keywords
	TOKEN_TYPE_FUNCTION_KEYWORD,  // 'fn'
	TOKEN_TYPE_RETURN_KEYWORD,    // 'return'
	TOKEN_TYPE_NAMESPACE_KEYWORD, // 'namespace'

	// Built-in types
	TOKEN_TYPE_INT_KEYWORD, // 'int'

	// Single-character token
	TOKEN_TYPE_OPEN_PAREN,  //  (
	TOKEN_TYPE_CLOSE_PAREN, //  )
	TOKEN_TYPE_OPEN_BRACE,  //  {
	TOKEN_TYPE_CLOSE_BRACE, //  }
	TOKEN_TYPE_SEMICOLON,   //  ;

	// Other
	TOKEN_TYPE_IDENTIFIER, //  ident
	TOKEN_TYPE_INTEGER,    // 123

	TOKEN_TYPE_EOF,
	TOKEN_TYPE_COUNT
} TokenType;

const char* token_type_to_cstr(TokenType type);

typedef struct Span
{
	u32 begin;
	u32 end;
} Span;

typedef struct Token
{
	TokenType type;
	Span span;
} Token;

typedef struct Lexer
{
	File* file_to_lex;

	Token cached_token;

	const char* current_char; // The current character being lexed
	u32 lexing_start_offset;
} Lexer;

Lexer lexer_create(File* file_to_lex);
void lexer_eat_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
Token lexer_peek_token_at_offset(Lexer* lexer, i32 offset);
void lexer_debug_print_token(Lexer* lexer, Token token);

void _lexer_advance(Lexer* lexer);
void _lexer_skip_whitespace(Lexer* lexer);
Token _lexer_create_new_token(Lexer* lexer, TokenType type);
Token _lexer_lex_identifier_or_keyword(Lexer* lexer);
Token _lexer_lex_digit(Lexer* lexer);
Token _lexer_lex_token(Lexer* lexer);
Token _lexer_lex_token_internal(Lexer* lexer);
