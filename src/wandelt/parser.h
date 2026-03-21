/**
 * @file parser.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast.h"
#include "wandelt/memory.h"
#include "wandelt/string.h"

typedef struct TranslationUnit
{
	const File* file;
	Statement** statements;
} TranslationUnit;

typedef struct Parser
{
	Allocator* stmt_allocator;
	Allocator* decl_allocator;
	Allocator* expr_allocator;

	Lexer* lexer;
	Token previous_token;
	TranslationUnit translation_unit;
} Parser;

Parser parser_create(Allocator* stmt_allocator, Allocator* decl_allocator, Allocator* expr_allocator, Lexer* lexer);

TranslationUnit parser_parse(Parser* parser);
Token parser_peek_token(Parser* parser);
void parser_eat_token(Parser* parser);

typedef enum Precedence
{
	PRECEDENCE_NONE = 0,
	PRECEDENCE_ADDITIVE, // + -
	PRECEDENCE_MULTIPLY, // * /
	PRECEDENCE_PRIMARY,  // literals, identifiers
} Precedence;

typedef Expression* (*PrefixParseFn)(Parser* parser);
typedef Expression* (*InfixParseFn)(Parser* parser, Expression* left);

typedef struct ParseRule
{
	PrefixParseFn prefix; // null denotation, prefix
	InfixParseFn infix;   // left denotation, infix/postfix
	Precedence precedence;
} ParseRule;

Statement* parser_parse_top_level_statement(Parser* parser);
void parser_recover_from_error(Parser* parser);

Statement* parser_parse_statement(Parser* parser);
Statement* parser_parse_declaration_statement(Parser* parser);
Statement* parser_parse_expression_statement(Parser* parser);
Statement* parser_parse_return_statement(Parser* parser);

Declaration* parser_parse_declaration(Parser* parser);
Declaration* parser_parse_namespace_declaration(Parser* parser);
Declaration* parser_parse_variable_declaration(Parser* parser);

Expression* parser_parse_expression(Parser* parser);
Expression* parser_parse_expression_with_precedence(Parser* parser, Precedence min_precedence);
Expression* parser_parse_constant_expression(Parser* parser);
Expression* parser_parse_binary_expression(Parser* parser, Expression* left);
Expression* parser_parse_identifier_expression(Parser* parser);

bool parser_parse_token(Parser* parser, TokenType expected_type);
bool parser_parse_identifier(Parser* parser, StringView* out_identifier);
bool parser_parse_type(Parser* parser, Type** out_type);
