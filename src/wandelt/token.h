/**
 * @file token.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"

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
	TOKEN_TYPE_PLUS,        //  +
	TOKEN_TYPE_MINUS,       //  -
	TOKEN_TYPE_STAR,        //  *
	TOKEN_TYPE_SLASH,       //  /

	// Other
	TOKEN_TYPE_IDENTIFIER, //  ident
	TOKEN_TYPE_INTEGER,    // 123

	TOKEN_TYPE_EOF,
	TOKEN_TYPE_COUNT
} TokenType;

const char* token_type_to_cstr(TokenType type);
const char* token_type_to_lexeme_cstr(TokenType type);

typedef struct Span
{
	u32 begin;
	u32 end;
} Span;

Span span_extend(Span a, Span b);

typedef struct Token
{
	TokenType type;
	Span span;
} Token;
