/**
 * @file lexer.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/file.h"
#include "wandelt/string.h"

typedef enum TokenType
{
	TOKEN_TYPE_INVALID = 0,

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

typedef struct SourceLocation
{
	u32 start_row;
	u32 start_col;
	u32 end_row;
	u32 end_col;
	String filename;
} SourceLocation;

typedef struct Token
{
	TokenType type;
	SourceLocation source_location;
	StringView lexeme;
} Token;

typedef struct Lexer
{
	File* file_to_lex;

	Token cached_token;

	const char* current_char;    // The current character being lexed
	const char* lexing_start;    // The start of the current token being lexed
	const char* line_start_char; // The start of the current line being lexed

	u32 current_row;
	u32 lexing_start_row;
} Lexer;

Lexer lexer_create(File* file_to_lex);
void lexer_eat_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
Token lexer_peek_token_at_offset(Lexer* lexer, i32 offset);

void _lexer_advance(Lexer* lexer);
void _lexer_skip_whitespace(Lexer* lexer);
Token _lexer_create_new_token(Lexer* lexer, TokenType type, StringView lexeme);
Token _lexer_lex_identifier_or_keyword(Lexer* lexer);
Token _lexer_lex_digit(Lexer* lexer);
Token _lexer_lex_token(Lexer* lexer);
Token _lexer_lex_token_internal(Lexer* lexer);
