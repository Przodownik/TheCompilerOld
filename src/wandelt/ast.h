/**
 * @file ast.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"
#include "wandelt/string.h"
#include "wandelt/token.h"
#include "wandelt/type.h"

typedef enum ResolveStatus
{
	RESOLVE_STATUS_UNRESOLVED = 0,
	RESOLVE_STATUS_RESOLVING,
	RESOLVE_STATUS_RESOLVED,
} ResolveStatus;

const char* resolve_status_to_cstr(ResolveStatus status);

typedef enum ExpressionType
{
	EXPRESSION_TYPE_INVALID = 0,
	EXPRESSION_TYPE_CONSTANT,
	EXPRESSION_TYPE_BINARY,
	EXPRESSION_TYPE_GROUP,
	EXPRESSION_TYPE_IDENTIFIER,
	EXPRESSION_TYPE_CAST,
	EXPRESSION_TYPE_COUNT,
} ExpressionType;

const char* expression_type_to_cstr(ExpressionType type);

typedef enum ConstantKind
{
	CONSTANT_KIND_INVALID = 0,
	CONSTANT_KIND_INTEGER,
	CONSTANT_KIND_FLOAT,
	CONSTANT_KIND_DOUBLE,
	CONSTANT_KIND_BOOLEAN,
	CONSTANT_KIND_COUNT,
} ConstantKind;

const char* constant_kind_to_cstr(ConstantKind kind);

typedef struct
{
	ConstantKind kind;

	union {
		u64 integer_value;
		float float_value;
		double double_value;
		bool boolean_value;
	};
} ConstantExpression;

typedef enum BinaryOperator
{
	BINARY_OPERATOR_INVALID = 0,
	BINARY_OPERATOR_ADD, // +
	BINARY_OPERATOR_SUB, // -
	BINARY_OPERATOR_MUL, // *
	BINARY_OPERATOR_DIV, // /
	BINARY_OPERATOR_COUNT,
} BinaryOperator;

const char* binary_operator_to_cstr(BinaryOperator op);
BinaryOperator token_type_to_binary_operator(TokenType type);

typedef struct BinaryExpression
{
	BinaryOperator operator;
	struct Expression* left;
	struct Expression* right;
} BinaryExpression;

typedef struct GroupExpression
{
	struct Expression* inner;
} GroupExpression;

typedef struct IdentifierExpression
{
	StringView name;
	struct Declaration* declaration_ref; // might be null
} IdentifierExpression;

typedef struct CastExpression
{
	Type* target_type;
	struct Expression* expression;
} CastExpression;

typedef struct Expression
{
	ExpressionType type;
	Span span;

	ResolveStatus resolve_status;
	Type* resolved_type;

	union {
		ConstantExpression constant;
		BinaryExpression binary;
		GroupExpression group;
		IdentifierExpression identifier;
		CastExpression cast;
	};
} Expression;

typedef enum DeclarationType
{
	DECLARATION_TYPE_INVALID = 0,
	DECLARATION_TYPE_NAMESPACE,
	DECLARATION_TYPE_VARIABLE,
	DECLARATION_TYPE_COUNT,
} DeclarationType;

const char* declaration_type_to_cstr(DeclarationType type);

typedef struct NamespaceDeclaration
{
	StringView name;
} NamespaceDeclaration;

typedef struct VariableDeclaration
{
	StringView name;
	Type* type;
	Expression* initializer;
} VariableDeclaration;

typedef struct Declaration
{
	DeclarationType type;
	Span span;
	ResolveStatus resolve_status;

	union {
		NamespaceDeclaration namespace;
		VariableDeclaration variable;
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
