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
	EXPRESSION_TYPE_UNARY,
	EXPRESSION_TYPE_BINARY,
	EXPRESSION_TYPE_GROUP,
	EXPRESSION_TYPE_IDENTIFIER,
	EXPRESSION_TYPE_CAST,
	EXPRESSION_TYPE_INCDEC,
	EXPRESSION_TYPE_CALL,
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
	BINARY_OPERATOR_EQ,  // ==
	BINARY_OPERATOR_NEQ, // !=
	BINARY_OPERATOR_LT,  // <
	BINARY_OPERATOR_GT,  // >
	BINARY_OPERATOR_LEQ, // <=
	BINARY_OPERATOR_GEQ, // >=
	BINARY_OPERATOR_COUNT,
} BinaryOperator;

const char* binary_operator_to_cstr(BinaryOperator op);
const char* binary_operator_to_token_cstr(BinaryOperator op);
BinaryOperator token_type_to_binary_operator(TokenType type);
bool binary_operator_is_comparison(BinaryOperator op);
bool binary_operator_is_equality(BinaryOperator op);
bool binary_operator_is_ordering(BinaryOperator op);

typedef enum UnaryOperator
{
	UNARY_OPERATOR_INVALID = 0,
	UNARY_OPERATOR_NEGATE, // -x
	UNARY_OPERATOR_COUNT,
} UnaryOperator;

const char* unary_operator_to_cstr(UnaryOperator op);

typedef struct UnaryExpression
{
	UnaryOperator operator;
	struct Expression* operand;
} UnaryExpression;

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

typedef enum AssignmentOperator
{
	ASSIGNMENT_OPERATOR_INVALID = 0,
	ASSIGNMENT_OPERATOR_PURE, // =
	ASSIGNMENT_OPERATOR_ADD,  // +=
	ASSIGNMENT_OPERATOR_SUB,  // -=
	ASSIGNMENT_OPERATOR_MUL,  // *=
	ASSIGNMENT_OPERATOR_DIV,  // /=
	ASSIGNMENT_OPERATOR_COUNT,
} AssignmentOperator;

const char* assignment_operator_to_cstr(AssignmentOperator op);
const char* assignment_operator_to_token_cstr(AssignmentOperator op);
AssignmentOperator token_type_to_assignment_operator(TokenType type);
BinaryOperator assignment_operator_to_binary_operator(AssignmentOperator op);

typedef struct IncDecExpression
{
	struct Expression* operand;
	bool is_increment;
	bool is_postfix;
} IncDecExpression;

typedef struct CallArgument
{
	struct Expression* value; // might be nullptr if default value is expected
} CallArgument;

typedef struct CallExpression
{
	StringView function_name;
	struct Declaration* declaration_ref;
	CallArgument* arguments;
} CallExpression;

typedef struct Expression
{
	ExpressionType type;
	Span span;

	ResolveStatus resolve_status;
	Type* resolved_type;

	union {
		ConstantExpression constant;
		UnaryExpression unary;
		BinaryExpression binary;
		GroupExpression group;
		IdentifierExpression identifier;
		CastExpression cast;
		IncDecExpression incdec;
		CallExpression call;
	};
} Expression;

typedef enum DeclarationType
{
	DECLARATION_TYPE_INVALID = 0,
	DECLARATION_TYPE_NAMESPACE,
	DECLARATION_TYPE_VARIABLE,
	DECLARATION_TYPE_FUNCTION,
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
	bool is_ever_assigned;
} VariableDeclaration;

typedef struct FunctionParameter
{
	StringView name;
	Type* type;
	Expression* default_value; // might be null if no default provided
} FunctionParameter;

typedef struct FunctionDeclaration
{
	StringView name;
	Type* return_type;
	FunctionParameter* parameters;
	struct Statement* body;
} FunctionDeclaration;

typedef struct Declaration
{
	DeclarationType type;
	Span span;
	ResolveStatus resolve_status;

	union {
		NamespaceDeclaration namespace;
		VariableDeclaration variable;
		FunctionDeclaration fn;
	};
} Declaration;

typedef enum StatementType
{
	STATEMENT_TYPE_INVALID = 0,
	STATEMENT_TYPE_DECLARATION,
	STATEMENT_TYPE_EXPRESSION,
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_BLOCK,
	STATEMENT_TYPE_IF,
	STATEMENT_TYPE_FOR,
	STATEMENT_TYPE_WHILE,
	STATEMENT_TYPE_ASSIGNMENT,
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

typedef struct BlockStatement
{
	struct Statement** statements;
} BlockStatement;

typedef struct IfStatement
{
	Expression* condition;
	struct Statement* then_block;
	struct Statement* else_block;
} IfStatement;

typedef struct ForStatement
{
	Declaration* initializer;
	Expression* condition;
	Expression* update;
	struct Statement* body;
	bool is_inline;
} ForStatement;

typedef struct WhileStatement
{
	Expression* condition;
	struct Statement* body;
} WhileStatement;

typedef struct AssignmentStatement
{
	AssignmentOperator operator;
	StringView target_identifier;
	Declaration* target_decl_ref;
	Type* target_type;
	Expression* value;
} AssignmentStatement;

typedef struct Statement
{
	StatementType type;
	Span span;
	struct Statement* next;

	union {
		DeclarationStatement decl_stmt;
		ExpressionStatement expr_stmt;
		ReturnStatement return_stmt;
		BlockStatement block_stmt;
		IfStatement if_stmt;
		ForStatement for_stmt;
		WhileStatement while_stmt;
		AssignmentStatement assign_stmt;
	};
} Statement;

void ast_dump_statements(Statement** statements);

typedef struct AstCopyContext
{
	Allocator* stmt_alloc;
	Allocator* expr_alloc;
	Allocator* decl_alloc;
	Declaration* subst_decl;
	i64 subst_value;
	Type* subst_type;
	Declaration* remap_old; // remap declaration_ref pointers (old -> new) without substituting to constant
	Declaration* remap_new;
} AstCopyContext;

Statement* ast_deep_copy_statement(AstCopyContext* ctx, const Statement* stmt);
Declaration* ast_deep_copy_declaration(AstCopyContext* ctx, const Declaration* decl);
Expression* ast_deep_copy_expression(AstCopyContext* ctx, const Expression* expr);
