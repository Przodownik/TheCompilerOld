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
	TOKEN_TYPE_VAR_KEYWORD,       // 'var'
	TOKEN_TYPE_AS_KEYWORD,        // 'as'

	// Built-in types
	TOKEN_TYPE_BOOL_KEYWORD,   // 'bool'
	TOKEN_TYPE_CHAR_KEYWORD,   // 'char'
	TOKEN_TYPE_UCHAR_KEYWORD,  // 'uchar'
	TOKEN_TYPE_SHORT_KEYWORD,  // 'short'
	TOKEN_TYPE_USHORT_KEYWORD, // 'ushort'
	TOKEN_TYPE_INT_KEYWORD,    // 'int'
	TOKEN_TYPE_UINT_KEYWORD,   // 'uint'
	TOKEN_TYPE_LONG_KEYWORD,   // 'long'
	TOKEN_TYPE_ULONG_KEYWORD,  // 'ulong'
	TOKEN_TYPE_FLOAT_KEYWORD,  // 'float'
	TOKEN_TYPE_DOUBLE_KEYWORD, // 'double'
	TOKEN_TYPE_TRUE_KEYWORD,   // 'true'
	TOKEN_TYPE_FALSE_KEYWORD,  // 'false'

	// Single-character tokens
	TOKEN_TYPE_OPEN_PAREN,  //  (
	TOKEN_TYPE_CLOSE_PAREN, //  )
	TOKEN_TYPE_OPEN_BRACE,  //  {
	TOKEN_TYPE_CLOSE_BRACE, //  }
	TOKEN_TYPE_SEMICOLON,   //  ;
	TOKEN_TYPE_PLUS,        //  +
	TOKEN_TYPE_MINUS,       //  -
	TOKEN_TYPE_STAR,        //  *
	TOKEN_TYPE_SLASH,       //  /
	TOKEN_TYPE_EQUALS,      //  =
	TOKEN_TYPE_DOT,         //  .
	TOKEN_TYPE_GREATER,     //  >
	TOKEN_TYPE_LESS,        //  <

	// Double-character tokens
	TOKEN_TYPE_GREATER_EQUAL, //  >=
	TOKEN_TYPE_LESS_EQUAL,    //  <=
	TOKEN_TYPE_EQUAL_EQUAL,   //  ==
	TOKEN_TYPE_BANG_EQUAL,    //  !=

	// Other
	TOKEN_TYPE_IDENTIFIER, //  ident
	TOKEN_TYPE_INTEGER,    // 123
	TOKEN_TYPE_FLOAT,      // 3.14f, 3.f, .14f
	TOKEN_TYPE_DOUBLE,     // 3.14d, 3.d, .14d

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
