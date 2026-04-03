#include "ast.h"
#include "wandelt/vector.h"

#include <stdio.h>

const char* resolve_status_to_cstr(ResolveStatus status)
{
	switch (status)
	{
	case RESOLVE_STATUS_UNRESOLVED:
		return "Unresolved";
	case RESOLVE_STATUS_RESOLVING:
		return "Resolving";
	case RESOLVE_STATUS_RESOLVED:
		return "Resolved";
	default:
		break;
	}

	ASSERT(false, "Unknown resolve status");
}

const char* expression_type_to_cstr(ExpressionType type)
{
	static_assert(EXPRESSION_TYPE_COUNT == 8, "Update expression_type_to_cstr when adding new expression types");

	switch (type)
	{
	case EXPRESSION_TYPE_INVALID:
		return "InvalidExpression";
	case EXPRESSION_TYPE_CONSTANT:
		return "ConstantExpression";
	case EXPRESSION_TYPE_UNARY:
		return "UnaryExpression";
	case EXPRESSION_TYPE_BINARY:
		return "BinaryExpression";
	case EXPRESSION_TYPE_GROUP:
		return "GroupExpression";
	case EXPRESSION_TYPE_IDENTIFIER:
		return "IdentifierExpression";
	case EXPRESSION_TYPE_CAST:
		return "CastExpression";
	case EXPRESSION_TYPE_INCDEC:
		return "IncDecExpression";
	default:
		break;
	}

	ASSERT(false, "Unknown expression type");
}

const char* constant_kind_to_cstr(ConstantKind kind)
{
	static_assert(CONSTANT_KIND_COUNT == 5, "Update constant_kind_to_cstr when adding new constant kinds");

	switch (kind)
	{
	case CONSTANT_KIND_INVALID:
		return "InvalidConstant";
	case CONSTANT_KIND_INTEGER:
		return "IntegerConstant";
	case CONSTANT_KIND_FLOAT:
		return "FloatConstant";
	case CONSTANT_KIND_DOUBLE:
		return "DoubleConstant";
	case CONSTANT_KIND_BOOLEAN:
		return "BooleanConstant";
	default:
		break;
	}

	ASSERT(false, "Unknown constant kind");
}

const char* binary_operator_to_cstr(BinaryOperator op)
{
	static_assert(BINARY_OPERATOR_COUNT == 11, "Update binary_operator_to_cstr when adding new binary operators");

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
	case BINARY_OPERATOR_EQ:
		return "Equal";
	case BINARY_OPERATOR_NEQ:
		return "NotEqual";
	case BINARY_OPERATOR_LT:
		return "LessThan";
	case BINARY_OPERATOR_GT:
		return "GreaterThan";
	case BINARY_OPERATOR_LEQ:
		return "LessEqual";
	case BINARY_OPERATOR_GEQ:
		return "GreaterEqual";
	default:
		break;
	}

	ASSERT(false, "Unknown binary operator");
}

const char* binary_operator_to_token_cstr(BinaryOperator op)
{
	static_assert(BINARY_OPERATOR_COUNT == 11, "Update binary_operator_to_token_cstr when adding new binary operators");
	switch (op)
	{
	case BINARY_OPERATOR_INVALID:
		return "InvalidBinaryOperator";
	case BINARY_OPERATOR_ADD:
		return "+";
	case BINARY_OPERATOR_SUB:
		return "-";
	case BINARY_OPERATOR_MUL:
		return "*";
	case BINARY_OPERATOR_DIV:
		return "/";
	case BINARY_OPERATOR_EQ:
		return "==";
	case BINARY_OPERATOR_NEQ:
		return "!=";
	case BINARY_OPERATOR_LT:
		return "<";
	case BINARY_OPERATOR_GT:
		return ">";
	case BINARY_OPERATOR_LEQ:
		return "<=";
	case BINARY_OPERATOR_GEQ:
		return ">=";
	default:
		break;
	}
	ASSERT(false, "Unknown binary operator");
}

BinaryOperator token_type_to_binary_operator(TokenType type)
{
	static_assert(BINARY_OPERATOR_COUNT == 11, "Update token_type_to_binary_operator when adding new binary operators");

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
	case TOKEN_TYPE_EQUAL_EQUAL:
		return BINARY_OPERATOR_EQ;
	case TOKEN_TYPE_BANG_EQUAL:
		return BINARY_OPERATOR_NEQ;
	case TOKEN_TYPE_LESS:
		return BINARY_OPERATOR_LT;
	case TOKEN_TYPE_GREATER:
		return BINARY_OPERATOR_GT;
	case TOKEN_TYPE_LESS_EQUAL:
		return BINARY_OPERATOR_LEQ;
	case TOKEN_TYPE_GREATER_EQUAL:
		return BINARY_OPERATOR_GEQ;
	default:
		break;
	}

	ASSERT(false, "Token type '%s' is not a binary operator", token_type_to_cstr(type));
}

bool binary_operator_is_comparison(BinaryOperator op)
{
	return op >= BINARY_OPERATOR_EQ && op <= BINARY_OPERATOR_GEQ;
}

bool binary_operator_is_equality(BinaryOperator op)
{
	return op == BINARY_OPERATOR_EQ || op == BINARY_OPERATOR_NEQ;
}

bool binary_operator_is_ordering(BinaryOperator op)
{
	return op == BINARY_OPERATOR_LT || op == BINARY_OPERATOR_GT || op == BINARY_OPERATOR_LEQ ||
	       op == BINARY_OPERATOR_GEQ;
}

const char* unary_operator_to_cstr(UnaryOperator op)
{
	static_assert(UNARY_OPERATOR_COUNT == 2, "Update unary_operator_to_cstr when adding new unary operators");

	switch (op)
	{
	case UNARY_OPERATOR_INVALID:
		return "InvalidUnaryOperator";
	case UNARY_OPERATOR_NEGATE:
		return "Negate";
	default:
		break;
	}
	ASSERT(false, "Unknown unary operator");
}

const char* assignment_operator_to_cstr(AssignmentOperator op)
{
	static_assert(ASSIGNMENT_OPERATOR_COUNT == 6, "Update assignment_operator_to_cstr");

	switch (op)
	{
	case ASSIGNMENT_OPERATOR_INVALID:
		return "InvalidAssignmentOperator";
	case ASSIGNMENT_OPERATOR_PURE:
		return "Assign";
	case ASSIGNMENT_OPERATOR_ADD:
		return "AddAssign";
	case ASSIGNMENT_OPERATOR_SUB:
		return "SubAssign";
	case ASSIGNMENT_OPERATOR_MUL:
		return "MulAssign";
	case ASSIGNMENT_OPERATOR_DIV:
		return "DivAssign";
	default:
		break;
	}

	ASSERT(false, "Unknown assignment operator");
}

const char* assignment_operator_to_token_cstr(AssignmentOperator op)
{
	static_assert(ASSIGNMENT_OPERATOR_COUNT == 6, "Update assignment_operator_to_token_cstr");

	switch (op)
	{
	case ASSIGNMENT_OPERATOR_INVALID:
		return "???";
	case ASSIGNMENT_OPERATOR_PURE:
		return "=";
	case ASSIGNMENT_OPERATOR_ADD:
		return "+=";
	case ASSIGNMENT_OPERATOR_SUB:
		return "-=";
	case ASSIGNMENT_OPERATOR_MUL:
		return "*=";
	case ASSIGNMENT_OPERATOR_DIV:
		return "/=";
	default:
		break;
	}

	ASSERT(false, "Unknown assignment operator");
}

AssignmentOperator token_type_to_assignment_operator(TokenType type)
{
	static_assert(ASSIGNMENT_OPERATOR_COUNT == 6, "Update token_type_to_assignment_operator");

	switch (type)
	{
	case TOKEN_TYPE_EQUALS:
		return ASSIGNMENT_OPERATOR_PURE;
	case TOKEN_TYPE_PLUS_EQUAL:
		return ASSIGNMENT_OPERATOR_ADD;
	case TOKEN_TYPE_MINUS_EQUAL:
		return ASSIGNMENT_OPERATOR_SUB;
	case TOKEN_TYPE_STAR_EQUAL:
		return ASSIGNMENT_OPERATOR_MUL;
	case TOKEN_TYPE_SLASH_EQUAL:
		return ASSIGNMENT_OPERATOR_DIV;
	default:
		break;
	}

	ASSERT(false, "Token '%s' is not an assignment operator", token_type_to_cstr(type));
}

BinaryOperator assignment_operator_to_binary_operator(AssignmentOperator op)
{
	static_assert(ASSIGNMENT_OPERATOR_COUNT == 6, "Update assignment_operator_to_binary_operator");

	switch (op)
	{
	case ASSIGNMENT_OPERATOR_ADD:
		return BINARY_OPERATOR_ADD;
	case ASSIGNMENT_OPERATOR_SUB:
		return BINARY_OPERATOR_SUB;
	case ASSIGNMENT_OPERATOR_MUL:
		return BINARY_OPERATOR_MUL;
	case ASSIGNMENT_OPERATOR_DIV:
		return BINARY_OPERATOR_DIV;
	default:
		break;
	}

	ASSERT(false, "Assignment operator '%s' has no binary equivalent", assignment_operator_to_cstr(op));
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
	static_assert(STATEMENT_TYPE_COUNT == 8, "Update statement_type_to_cstr when adding new statement types");

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

	case STATEMENT_TYPE_BLOCK:
		return "BlockStatement";

	case STATEMENT_TYPE_IF:
		return "IfStatement";

	case STATEMENT_TYPE_WHILE:
		return "WhileStatement";

	case STATEMENT_TYPE_ASSIGNMENT:
		return "AssignmentStatement";

	default:
		break;
	}

	ASSERT(false, "Unknown statement type");
}

static void dump_expression(Expression* expr, int indent)
{
	static_assert(EXPRESSION_TYPE_COUNT == 8, "Update dump_expression when adding new expression types");

	if (!expr)
		return;

	printf("%*sExpression: %s\n", indent, "", expression_type_to_cstr(expr->type));
	printf("%*sResolve status: %s\n", indent + 2, "", resolve_status_to_cstr(expr->resolve_status));
	printf("%*sType: %s\n", indent + 2, "",
	       expr->resolved_type ? type_kind_to_cstr(expr->resolved_type->kind) : "Unresolved");

	if (expr->type == EXPRESSION_TYPE_CONSTANT)
	{
		printf("%*sConstant kind: %s\n", indent + 2, "", constant_kind_to_cstr(expr->constant.kind));
		if (expr->constant.kind == CONSTANT_KIND_INTEGER)
		{
			if ((i64)expr->constant.integer_value < 0)
				printf("%*sValue: %lld\n", indent + 2, "", (i64)expr->constant.integer_value);
			else
				printf("%*sValue: %llu\n", indent + 2, "", expr->constant.integer_value);
		}
		else if (expr->constant.kind == CONSTANT_KIND_FLOAT)
			printf("%*sValue: %f\n", indent + 2, "", (double)expr->constant.float_value);
		else if (expr->constant.kind == CONSTANT_KIND_DOUBLE)
			printf("%*sValue: %lf\n", indent + 2, "", expr->constant.double_value);
		else if (expr->constant.kind == CONSTANT_KIND_BOOLEAN)
			printf("%*sValue: %s\n", indent + 2, "", expr->constant.boolean_value ? "true" : "false");
	}
	else if (expr->type == EXPRESSION_TYPE_UNARY)
	{
		printf("%*sUnary operator: %s\n", indent + 2, "", unary_operator_to_cstr(expr->unary.operator));
		printf("%*sOperand:\n", indent + 2, "");
		dump_expression(expr->unary.operand, indent + 4);
	}
	else if (expr->type == EXPRESSION_TYPE_BINARY)
	{
		printf("%*sBinary operator: %s\n", indent + 2, "", binary_operator_to_cstr(expr->binary.operator));
		printf("%*sLeft:\n", indent + 2, "");
		dump_expression(expr->binary.left, indent + 4);
		printf("%*sRight:\n", indent + 2, "");
		dump_expression(expr->binary.right, indent + 4);
	}
	else if (expr->type == EXPRESSION_TYPE_GROUP)
	{
		printf("%*sInner expression:\n", indent + 2, "");
		dump_expression(expr->group.inner, indent + 4);
	}
	else if (expr->type == EXPRESSION_TYPE_IDENTIFIER)
	{
		printf("%*sIdentifier: %.*s\n", indent + 2, "", FMT_STR_ARG(expr->identifier.name));
	}
	else if (expr->type == EXPRESSION_TYPE_CAST)
	{
		printf("%*sCast to type: %s\n", indent + 2, "", type_kind_to_cstr(expr->cast.target_type->kind));
		printf("%*sExpression:\n", indent + 2, "");
		dump_expression(expr->cast.expression, indent + 4);
	}
	else if (expr->type == EXPRESSION_TYPE_INCDEC)
	{
		printf("%*s%s %s\n", indent + 2, "", expr->incdec.is_increment ? "Increment" : "Decrement",
		       expr->incdec.is_postfix ? "(postfix)" : "(prefix)");
		printf("%*sOperand:\n", indent + 2, "");
		dump_expression(expr->incdec.operand, indent + 4);
	}
	else
	{
		ASSERT(false);
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
	else
	{
		ASSERT(false);
	}
}

static void dump_statement(Statement* stmt, int indent)
{
	static_assert(STATEMENT_TYPE_COUNT == 8, "Update dump_statement when adding new statement types");

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
	case STATEMENT_TYPE_BLOCK:
		printf("%*sStatements:\n", indent + 2, "");
		for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
		{
			printf("%*s[%llu] ", indent + 4, "", i);
			dump_statement(stmt->block_stmt.statements[i], indent + 6);
		}
		break;
	case STATEMENT_TYPE_IF:
		printf("%*sCondition:\n", indent + 2, "");
		dump_expression(stmt->if_stmt.condition, indent + 4);
		printf("%*sThen branch:\n", indent + 2, "");
		dump_statement(stmt->if_stmt.then_block, indent + 4);
		if (stmt->if_stmt.else_block)
		{
			printf("%*sElse branch:\n", indent + 2, "");
			dump_statement(stmt->if_stmt.else_block, indent + 4);
		}
		break;
	case STATEMENT_TYPE_WHILE:
		printf("%*sCondition:\n", indent + 2, "");
		dump_expression(stmt->while_stmt.condition, indent + 4);
		printf("%*sBody:\n", indent + 2, "");
		dump_statement(stmt->while_stmt.body, indent + 4);
		break;
	case STATEMENT_TYPE_ASSIGNMENT:
		printf("%*sAssignment: %s\n", indent + 2, "", assignment_operator_to_cstr(stmt->assign_stmt.operator));
		printf("%*sTarget: %.*s\n", indent + 2, "", FMT_STR_ARG(stmt->assign_stmt.target_identifier));
		printf("%*sValue:\n", indent + 2, "");
		dump_expression(stmt->assign_stmt.value, indent + 4);
		break;
	default:
		ASSERT(false);
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
