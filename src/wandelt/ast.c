#include "ast.h"
#include "wandelt/vector.h"

#include <stdio.h>

const char* expression_type_to_cstr(ExpressionType type)
{
	static_assert(EXPRESSION_TYPE_COUNT == 4, "Update expression_type_to_cstr when adding new expression types");

	switch (type)
	{
	case EXPRESSION_TYPE_INVALID:
		return "InvalidExpression";
	case EXPRESSION_TYPE_CONSTANT:
		return "ConstantExpression";
	case EXPRESSION_TYPE_BINARY:
		return "BinaryExpression";
	case EXPRESSION_TYPE_IDENTIFIER:
		return "IdentifierExpression";
	default:
		break;
	}

	ASSERT(false, "Unknown expression type");
}

const char* constant_kind_to_cstr(ConstantKind kind)
{
	static_assert(CONSTANT_KIND_COUNT == 2, "Update constant_kind_to_cstr when adding new constant kinds");

	switch (kind)
	{
	case CONSTANT_KIND_INVALID:
		return "InvalidConstant";
	case CONSTANT_KIND_INTEGER:
		return "IntegerConstant";
	default:
		break;
	}

	ASSERT(false, "Unknown constant kind");
}

const char* binary_operator_to_cstr(BinaryOperator op)
{
	static_assert(BINARY_OPERATOR_COUNT == 5, "Update binary_operator_to_cstr when adding new binary operators");

	switch (op)
	{
	case BINARY_OPERATOR_INVALID:
		return "InvalidBinaryOperator";
	case BINARY_OPERATOR_ADD:
		return "Add";
	case BINARY_OPERATOR_SUB:
		return "Subtract";
	case BINARY_OPERATOR_MUL:
		return "Multiply";
	case BINARY_OPERATOR_DIV:
		return "Divide";
	default:
		break;
	}

	ASSERT(false, "Unknown binary operator");
}

BinaryOperator token_type_to_binary_operator(TokenType type)
{
	static_assert(BINARY_OPERATOR_COUNT == 5, "Update token_type_to_binary_operator when adding new binary operators");

	switch (type)
	{
	case TOKEN_TYPE_PLUS:
		return BINARY_OPERATOR_ADD;
	case TOKEN_TYPE_MINUS:
		return BINARY_OPERATOR_SUB;
	case TOKEN_TYPE_STAR:
		return BINARY_OPERATOR_MUL;
	case TOKEN_TYPE_SLASH:
		return BINARY_OPERATOR_DIV;
	default:
		break;
	}

	ASSERT(false, "Token type '%s' is not a binary operator", token_type_to_cstr(type));
}

const char* declaration_type_to_cstr(DeclarationType type)
{
	static_assert(DECLARATION_TYPE_COUNT == 3, "Update declaration_type_to_cstr when adding new declaration types");

	switch (type)
	{
	case DECLARATION_TYPE_INVALID:
		return "InvalidDeclaration";
	case DECLARATION_TYPE_NAMESPACE:
		return "NamespaceDeclaration";
	case DECLARATION_TYPE_VARIABLE:
		return "VariableDeclaration";
	default:
		break;
	}

	ASSERT(false, "Unknown declaration type");
}

const char* statement_type_to_cstr(StatementType type)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update statement_type_to_cstr when adding new statement types");

	switch (type)
	{
	case STATEMENT_TYPE_INVALID:
		return "InvalidStatement";
	case STATEMENT_TYPE_DECLARATION:
		return "DeclarationStatement";
	case STATEMENT_TYPE_EXPRESSION:
		return "ExpressionStatement";
	case STATEMENT_TYPE_RETURN:
		return "ReturnStatement";
	default:
		break;
	}

	ASSERT(false, "Unknown statement type");
}

static void dump_expression(Expression* expr, int indent)
{
	static_assert(EXPRESSION_TYPE_COUNT == 4, "Update dump_expression when adding new expression types");

	if (!expr)
		return;

	printf("%*sExpression: %s\n", indent, "", expression_type_to_cstr(expr->type));

	if (expr->type == EXPRESSION_TYPE_CONSTANT)
	{
		printf("%*sConstant kind: %s\n", indent + 2, "", constant_kind_to_cstr(expr->constant.kind));
		if (expr->constant.kind == CONSTANT_KIND_INTEGER)
			printf("%*sValue: %llu\n", indent + 2, "", expr->constant.integer);
	}
	else if (expr->type == EXPRESSION_TYPE_BINARY)
	{
		printf("%*sBinary operator: %s\n", indent + 2, "", binary_operator_to_cstr(expr->binary.operator));
		printf("%*sLeft:\n", indent + 2, "");
		dump_expression(expr->binary.left, indent + 4);
		printf("%*sRight:\n", indent + 2, "");
		dump_expression(expr->binary.right, indent + 4);
	}
	else if (expr->type == EXPRESSION_TYPE_IDENTIFIER)
	{
		printf("%*sIdentifier: %.*s\n", indent + 2, "", FMT_STR_ARG(expr->identifier.name));
	}
}

static void dump_declaration(Declaration* decl, int indent)
{
	static_assert(DECLARATION_TYPE_COUNT == 3, "Update dump_declaration when adding new declaration types");

	if (!decl)
		return;

	printf("%*sDeclaration: %s\n", indent, "", declaration_type_to_cstr(decl->type));

	if (decl->type == DECLARATION_TYPE_NAMESPACE)
		printf("%*sName: %.*s\n", indent + 2, "", FMT_STR_ARG(decl->namespace.name));
	else if (decl->type == DECLARATION_TYPE_VARIABLE)
	{
		printf("%*sName: %.*s\n", indent + 2, "", FMT_STR_ARG(decl->variable.name));
		printf("%*sType: %s\n", indent + 2, "", type_kind_to_cstr(decl->variable.type->kind));
		printf("%*sInitializer:\n", indent + 2, "");
		dump_expression(decl->variable.initializer, indent + 4);
	}
}

static void dump_statement(Statement* stmt, int indent)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update dump_statement when adding new statement types");

	if (!stmt)
		return;

	printf("%*sStatement: %s\n", indent, "", statement_type_to_cstr(stmt->type));

	switch (stmt->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		dump_declaration(stmt->decl_stmt.declaration, indent + 2);
		break;
	case STATEMENT_TYPE_EXPRESSION:
		dump_expression(stmt->expr_stmt.expression, indent + 2);
		break;
	case STATEMENT_TYPE_RETURN:
		dump_expression(stmt->return_stmt.expression, indent + 2);
		break;
	default:
		break;
	}
}

void ast_dump_statements(Statement** statements)
{
	printf("=== AST Dump (%llu statements) ===\n", vector_get_length(statements));

	for (u64 i = 0; i < vector_get_length(statements); i++)
	{
		printf("[%llu] ", i);
		dump_statement(statements[i], 0);
	}

	printf("=== End AST Dump ===\n");
}
