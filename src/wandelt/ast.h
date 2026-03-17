/**
 * @file ast.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"
#include "wandelt/lexer.h"

typedef enum ExpressionType
{
	EXPRESSION_TYPE_INVALID = 0,
	EXPRESSION_TYPE_CONSTANT,
	EXPRESSION_TYPE_COUNT,
} ExpressionType;

const char* expression_type_to_cstr(ExpressionType type);

typedef enum ConstantKind
{
	CONSTANT_KIND_INVALID = 0,
	CONSTANT_KIND_INTEGER,
	CONSTANT_KIND_COUNT,
} ConstantKind;

const char* constant_kind_to_cstr(ConstantKind kind);

typedef struct
{
	ConstantKind kind;

	union {
		u64 integer;
		// double float_value;
	};
} ConstantExpression;

typedef struct Expression
{
	ExpressionType type;
	Span span;

	union {
		ConstantExpression constant;
	};
} Expression;

typedef enum DeclarationType
{
	DECLARATION_TYPE_INVALID = 0,
	DECLARATION_TYPE_NAMESPACE,
	DECLARATION_TYPE_COUNT,
} DeclarationType;

const char* declaration_type_to_cstr(DeclarationType type);

typedef struct NamespaceDeclaration
{
	StringView name;
} NamespaceDeclaration;

typedef struct Declaration
{
	DeclarationType type;
	Span span;

	union {
		NamespaceDeclaration namespace;
	};
} Declaration;

typedef enum StatementType
{
	STATEMENT_TYPE_INVALID = 0,
	STATEMENT_TYPE_DECLARATION,
	STATEMENT_TYPE_EXPRESSION,
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_COUNT,
} StatementType;

const char* statement_type_to_cstr(StatementType type);

typedef struct DeclarationStatement
{
	Declaration* declaration;
} DeclarationStatement;

typedef struct ExpressionStatement
{
	Expression* expression;
} ExpressionStatement;

typedef struct ReturnStatement
{
	Expression* expression;
} ReturnStatement;

typedef struct Statement
{
	StatementType type;
	Span span;
	struct Statement* next;

	union {
		DeclarationStatement decl_stmt;
		ExpressionStatement expr_stmt;
		ReturnStatement return_stmt;
	};
} Statement;

void ast_dump_statements(Statement** statements);
