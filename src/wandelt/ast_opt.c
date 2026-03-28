#include "ast_opt.h"

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/type.h"
#include "wandelt/vector.h"

AstOptimizer ast_optimizer_create(Allocator* expr_alloc)
{
	return (AstOptimizer){
	    .expr_alloc = expr_alloc,
	};
}

void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu)
{
	bool changed = true;

	while (changed)
	{
		changed = false;

		for (u64 i = 0; i < vector_get_length(tu->statements); i++)
		{
			Statement* stmt = tu->statements[i];
			changed |= ast_optimizer_fold_statement(optimizer, stmt);
		}

		for (u64 i = 0; i < vector_get_length(tu->statements); i++)
		{
			Statement* stmt = tu->statements[i];
			changed |= ast_optimizer_propagate_statement(optimizer, stmt);
		}
	}

	ast_optimizer_dce(optimizer, tu);
}

bool ast_optimizer_fold_statement(AstOptimizer* optimizer, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update this function when adding new statement types");

	switch (stmt->type)
	{
	case STATEMENT_TYPE_INVALID:
		ASSERT(false, "Invalid statement.");
		break;

	case STATEMENT_TYPE_DECLARATION:
		return ast_optimizer_fold_declaration_statement(optimizer, stmt);

	case STATEMENT_TYPE_EXPRESSION:
		return ast_optimizer_fold_expression_statement(optimizer, stmt);

	case STATEMENT_TYPE_RETURN:
		return ast_optimizer_fold_return_statement(optimizer, stmt);

	case STATEMENT_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid statement.");
		break;
	}
}

bool ast_optimizer_fold_declaration_statement(AstOptimizer* optimizer, Statement* stmt)
{
	Declaration* decl = stmt->decl_stmt.declaration;

	if (decl->type == DECLARATION_TYPE_VARIABLE)
	{
		if (ast_optimizer_fold_expression(optimizer, &decl->variable.initializer))
			return true;
	}

	return false;
}

bool ast_optimizer_fold_expression_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_fold_expression(optimizer, &stmt->expr_stmt.expression);
}

bool ast_optimizer_fold_return_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_fold_expression(optimizer, &stmt->return_stmt.expression);
}

bool ast_optimizer_fold_expression(AstOptimizer* optimizer, Expression** expr)
{
	const Expression* expression = *expr;

	switch (expression->type)
	{
	case EXPRESSION_TYPE_INVALID:
		ASSERT(false, "Invalid statement.");
		break;

	case EXPRESSION_TYPE_CONSTANT:
		return ast_optimizer_fold_constant_expression(optimizer, expr);

	case EXPRESSION_TYPE_UNARY:
		return ast_optimizer_fold_unary_expression(optimizer, expr);

	case EXPRESSION_TYPE_BINARY:
		return ast_optimizer_fold_binary_expression(optimizer, expr);

	case EXPRESSION_TYPE_GROUP:
		return ast_optimizer_fold_group_expression(optimizer, expr);

	case EXPRESSION_TYPE_IDENTIFIER:
		return ast_optimizer_fold_identifier_expression(optimizer, expr);

	case EXPRESSION_TYPE_CAST:
		return ast_optimizer_fold_cast_expression(optimizer, expr);

	case EXPRESSION_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid expression.");
		break;
	}
}

bool ast_optimizer_fold_constant_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;
	(void)expr;

	return false;
}

bool ast_optimizer_fold_unary_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;
	Expression* operand    = expression->unary.operand;

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->unary.operand);

	if (operand->type != EXPRESSION_TYPE_CONSTANT)
		return changed;

	if (expression->unary.operator == UNARY_OPERATOR_NEGATE)
	{
		switch (operand->constant.kind)
		{
		case CONSTANT_KIND_INVALID:
			ASSERT(false, "Invalid constant.");
			break;

		case CONSTANT_KIND_INTEGER:
			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_INTEGER;
			expression->constant.integer_value = (u64)(-(i64)operand->constant.integer_value);
			return true;

		case CONSTANT_KIND_FLOAT:
			expression->type                 = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind        = CONSTANT_KIND_FLOAT;
			expression->constant.float_value = -operand->constant.float_value;
			return true;

		case CONSTANT_KIND_DOUBLE:
			expression->type                  = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind         = CONSTANT_KIND_DOUBLE;
			expression->constant.double_value = -operand->constant.double_value;
			return true;

		case CONSTANT_KIND_BOOLEAN:
			ASSERT(false, "CE: Cannot apply unary minus to boolean constant.");
			return changed;

		case CONSTANT_KIND_COUNT:
		default:
			ASSERT(false, "Invalid constant.");
			break;
		}
	}

	ASSERT(false, "TODO: Unsupported unary operator for constant folding.");

	return false;
}

bool ast_optimizer_fold_binary_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;
	Expression* left       = expression->binary.left;
	Expression* right      = expression->binary.right;

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->binary.left);
	changed |= ast_optimizer_fold_expression(optimizer, &expression->binary.right);

	if (left->type != EXPRESSION_TYPE_CONSTANT || right->type != EXPRESSION_TYPE_CONSTANT)
		return changed;

	ASSERT(left->constant.kind == right->constant.kind);

	switch (left->constant.kind)
	{
	case CONSTANT_KIND_INVALID:
		ASSERT(false, "Invalid constant.");
		break;

	case CONSTANT_KIND_INTEGER: {
		u64 lval = left->constant.integer_value;
		u64 rval = right->constant.integer_value;

		if (binary_operator_is_comparison(expression->binary.operator))
		{
			bool result = false;

			switch (expression->binary.operator)
			{
			case BINARY_OPERATOR_EQ:
				result = lval == rval;
				break;
			case BINARY_OPERATOR_NEQ:
				result = lval != rval;
				break;
			case BINARY_OPERATOR_LT:
				result = lval < rval;
				break;
			case BINARY_OPERATOR_GT:
				result = lval > rval;
				break;
			case BINARY_OPERATOR_LEQ:
				result = lval <= rval;
				break;
			case BINARY_OPERATOR_GEQ:
				result = lval >= rval;
				break;
			default:
				ASSERT(false, "Not a comparison operator");
				break;
			}

			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_BOOLEAN;
			expression->constant.boolean_value = result;

			return true;
		}

		u64 result;

		switch (expression->binary.operator)
		{
		case BINARY_OPERATOR_ADD:
			result = lval + rval;
			break;
		case BINARY_OPERATOR_SUB:
			result = lval - rval;
			break;
		case BINARY_OPERATOR_MUL:
			result = lval * rval;
			break;
		case BINARY_OPERATOR_DIV:
			ASSERT(rval != 0);
			result = lval / rval;
			break;
		default:
			ASSERT(false, "Not a supported binary operator for integer constant folding.");
			break;
		}

		expression->type                   = EXPRESSION_TYPE_CONSTANT;
		expression->constant.kind          = CONSTANT_KIND_INTEGER;
		expression->constant.integer_value = (u64)result;

		return true;
	}

	case CONSTANT_KIND_FLOAT: {
		float lval = left->constant.float_value;
		float rval = right->constant.float_value;

		if (binary_operator_is_comparison(expression->binary.operator))
		{
			bool result = false;
			switch (expression->binary.operator)
			{
			case BINARY_OPERATOR_EQ:
				result = lval == rval;
				break;
			case BINARY_OPERATOR_NEQ:
				result = lval != rval;
				break;
			case BINARY_OPERATOR_LT:
				result = lval < rval;
				break;
			case BINARY_OPERATOR_GT:
				result = lval > rval;
				break;
			case BINARY_OPERATOR_LEQ:
				result = lval <= rval;
				break;
			case BINARY_OPERATOR_GEQ:
				result = lval >= rval;
				break;
			default:
				ASSERT(false, "Not a comparison operator");
				break;
			}
			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_BOOLEAN;
			expression->constant.boolean_value = result;
			return true;
		}

		float result;

		switch (expression->binary.operator)
		{
		case BINARY_OPERATOR_ADD:
			result = lval + rval;
			break;
		case BINARY_OPERATOR_SUB:
			result = lval - rval;
			break;
		case BINARY_OPERATOR_MUL:
			result = lval * rval;
			break;
		case BINARY_OPERATOR_DIV:
			ASSERT(rval != 0.0f);
			result = lval / rval;
			break;
		default:
			ASSERT(false, "Not a supported binary operator for float constant folding.");
			break;
		}

		expression->type                 = EXPRESSION_TYPE_CONSTANT;
		expression->constant.kind        = CONSTANT_KIND_FLOAT;
		expression->constant.float_value = result;

		return true;
	}

	case CONSTANT_KIND_DOUBLE: {
		double lval = left->constant.double_value;
		double rval = right->constant.double_value;

		if (binary_operator_is_comparison(expression->binary.operator))
		{
			bool result = false;
			switch (expression->binary.operator)
			{
			case BINARY_OPERATOR_EQ:
				result = lval == rval;
				break;
			case BINARY_OPERATOR_NEQ:
				result = lval != rval;
				break;
			case BINARY_OPERATOR_LT:
				result = lval < rval;
				break;
			case BINARY_OPERATOR_GT:
				result = lval > rval;
				break;
			case BINARY_OPERATOR_LEQ:
				result = lval <= rval;
				break;
			case BINARY_OPERATOR_GEQ:
				result = lval >= rval;
				break;
			default:
				ASSERT(false, "Not a comparison operator");
				break;
			}
			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_BOOLEAN;
			expression->constant.boolean_value = result;
			return true;
		}

		double result;

		switch (expression->binary.operator)
		{
		case BINARY_OPERATOR_ADD:
			result = lval + rval;
			break;
		case BINARY_OPERATOR_SUB:
			result = lval - rval;
			break;
		case BINARY_OPERATOR_MUL:
			result = lval * rval;
			break;
		case BINARY_OPERATOR_DIV:
			ASSERT(rval != 0.0);
			result = lval / rval;
			break;
		default:
			ASSERT(false, "Not a supported binary operator for double constant folding.");
			break;
		}

		expression->type                  = EXPRESSION_TYPE_CONSTANT;
		expression->constant.kind         = CONSTANT_KIND_DOUBLE;
		expression->constant.double_value = result;

		return true;
	}

	case CONSTANT_KIND_BOOLEAN:
		ASSERT(false, "CE: sCannot apply unary minus to boolean constant.");
		return changed;

	case CONSTANT_KIND_COUNT:
	default:
		ASSERT(false, "Invalid constant.");
		break;
	}
}

bool ast_optimizer_fold_group_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->group.inner);
	if (expression->group.inner->type == EXPRESSION_TYPE_CONSTANT)
	{
		*expr = expression->group.inner;
		return true;
	}

	return changed;
}

bool ast_optimizer_fold_identifier_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;
	(void)expr;

	return false;
}

bool ast_optimizer_fold_cast_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;
	Expression* inner      = expression->cast.expression;

	bool changed = ast_optimizer_fold_expression(optimizer, &inner);

	if (expression->cast.expression->type == EXPRESSION_TYPE_CONSTANT)
	{
		Type* target    = expression->cast.target_type;
		ConstantKind ck = inner->constant.kind;

		i64 as_i64 = 0;
		f64 as_f64 = 0.0;

		switch (ck)
		{
		case CONSTANT_KIND_BOOLEAN:
			as_i64 = inner->constant.boolean_value ? 1 : 0;
			as_f64 = (f64)as_i64;
			break;
		case CONSTANT_KIND_INTEGER:
			as_i64 = (i64)inner->constant.integer_value;
			as_f64 = (f64)as_i64;
			break;
		case CONSTANT_KIND_FLOAT:
			as_f64 = (f64)inner->constant.float_value;
			as_i64 = (i64)as_f64;
			break;
		case CONSTANT_KIND_DOUBLE:
			as_f64 = inner->constant.double_value;
			as_i64 = (i64)as_f64;
			break;
		default:
			return expr;
		}

		expression->type = EXPRESSION_TYPE_CONSTANT;

		if (type_is_integer(target))
		{
			expression->constant.kind          = CONSTANT_KIND_INTEGER;
			expression->constant.integer_value = (u64)as_i64;
		}
		else if (target->kind == TYPE_KIND_FLOAT)
		{
			expression->constant.kind        = CONSTANT_KIND_FLOAT;
			expression->constant.float_value = (float)as_f64;
		}
		else if (target->kind == TYPE_KIND_DOUBLE)
		{
			expression->constant.kind         = CONSTANT_KIND_DOUBLE;
			expression->constant.double_value = as_f64;
		}
		else if (target->kind == TYPE_KIND_BOOL)
		{
			expression->constant.kind          = CONSTANT_KIND_BOOLEAN;
			expression->constant.boolean_value = as_i64 != 0;
		}
		else
		{
			ASSERT(false, "Unsupported target type for cast constant folding.");
		}

		expression->resolved_type = target;
	}

	return changed;
}

bool ast_optimizer_propagate_statement(AstOptimizer* optimizer, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update this function when adding new statement types");

	switch (stmt->type)
	{
	case STATEMENT_TYPE_INVALID:
		ASSERT(false, "Invalid statement.");
		break;

	case STATEMENT_TYPE_DECLARATION:
		return ast_optimizer_propagate_declaration_statement(optimizer, stmt);

	case STATEMENT_TYPE_EXPRESSION:
		return ast_optimizer_propagate_expression_statement(optimizer, stmt);

	case STATEMENT_TYPE_RETURN:
		return ast_optimizer_propagate_return_statement(optimizer, stmt);

	case STATEMENT_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid statement.");
		break;
	}
}

bool ast_optimizer_propagate_declaration_statement(AstOptimizer* optimizer, Statement* stmt)
{
	Declaration* decl = stmt->decl_stmt.declaration;

	if (decl->type == DECLARATION_TYPE_VARIABLE)
		return ast_optimizer_propagate_expression(optimizer, &decl->variable.initializer);

	return false;
}

bool ast_optimizer_propagate_expression_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_propagate_expression(optimizer, &stmt->expr_stmt.expression);
}

bool ast_optimizer_propagate_return_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_propagate_expression(optimizer, &stmt->return_stmt.expression);
}

bool ast_optimizer_propagate_expression(AstOptimizer* optimizer, Expression** expr)
{
	const Expression* expression = *expr;

	switch (expression->type)
	{
	case EXPRESSION_TYPE_INVALID:
		ASSERT(false, "Invalid expression.");
		break;

	case EXPRESSION_TYPE_CONSTANT:
		return ast_optimizer_propagate_constant_expression(optimizer, expr);

	case EXPRESSION_TYPE_UNARY:
		return ast_optimizer_propagate_unary_expression(optimizer, expr);

	case EXPRESSION_TYPE_BINARY:
		return ast_optimizer_propagate_binary_expression(optimizer, expr);

	case EXPRESSION_TYPE_GROUP:
		return ast_optimizer_propagate_group_expression(optimizer, expr);

	case EXPRESSION_TYPE_IDENTIFIER:
		return ast_optimizer_propagate_identifier_expression(optimizer, expr);

	case EXPRESSION_TYPE_CAST:
		return ast_optimizer_propagate_cast_expression(optimizer, expr);

	case EXPRESSION_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid expression.");
		break;
	}
}

bool ast_optimizer_propagate_constant_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;
	(void)expr;

	return false;
}

bool ast_optimizer_propagate_unary_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;
	return ast_optimizer_propagate_expression(optimizer, &expression->unary.operand);
}

bool ast_optimizer_propagate_binary_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;

	bool changed = ast_optimizer_propagate_expression(optimizer, &expression->binary.left);
	changed |= ast_optimizer_propagate_expression(optimizer, &expression->binary.right);

	return changed;
}

bool ast_optimizer_propagate_group_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;
	return ast_optimizer_propagate_expression(optimizer, &expression->group.inner);
}

bool ast_optimizer_propagate_identifier_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;

	Expression* expression = *expr;
	Declaration* decl      = expression->identifier.declaration_ref;
	ASSERT(decl, "Identifier expression has no declaration reference.");

	if (decl->type != DECLARATION_TYPE_VARIABLE)
		return false;

	Expression* init = decl->variable.initializer;
	if (init->type != EXPRESSION_TYPE_CONSTANT)
		return false;

	expression->type     = EXPRESSION_TYPE_CONSTANT;
	expression->constant = init->constant;

	return true;
}

bool ast_optimizer_propagate_cast_expression(AstOptimizer* optimizer, Expression** expr)
{
	Expression* expression = *expr;

	return ast_optimizer_propagate_expression(optimizer, &expression->cast.expression);
}

void ast_optimizer_dce(AstOptimizer* optimizer, TranslationUnit* tu)
{
	(void)optimizer;

	Allocator heap     = allocator_get_heap_allocator();
	Declaration** used = vector_create(&heap, 16, sizeof(Declaration*));

	for (u64 i = 0; i < vector_get_length(tu->statements); i++)
		ast_optimizer_dce_mark_statement(tu->statements[i], used);

	u64 i = 0;
	while (i < vector_get_length(tu->statements))
	{
		Statement* stmt = tu->statements[i];

		if (stmt->type == STATEMENT_TYPE_DECLARATION &&
		    stmt->decl_stmt.declaration->type == DECLARATION_TYPE_VARIABLE &&
		    !ast_optimizer_dce_is_used(stmt->decl_stmt.declaration, used))
		{
			vector_remove_at(tu->statements, i, NULL);
		}
		else
		{
			i++;
		}
	}

	vector_destroy(used);
}

void ast_optimizer_dce_mark_expression(Expression* expr, Declaration** used)
{
	switch (expr->type)
	{
	case EXPRESSION_TYPE_IDENTIFIER:
		if (expr->identifier.declaration_ref)
			vector_push(used, expr->identifier.declaration_ref);
		break;

	case EXPRESSION_TYPE_UNARY:
		ast_optimizer_dce_mark_expression(expr->unary.operand, used);
		break;

	case EXPRESSION_TYPE_BINARY:
		ast_optimizer_dce_mark_expression(expr->binary.left, used);
		ast_optimizer_dce_mark_expression(expr->binary.right, used);
		break;

	case EXPRESSION_TYPE_GROUP:
		ast_optimizer_dce_mark_expression(expr->group.inner, used);
		break;

	case EXPRESSION_TYPE_CAST:
		ast_optimizer_dce_mark_expression(expr->cast.expression, used);
		break;

	case EXPRESSION_TYPE_CONSTANT:
	default:
		break;
	}
}

void ast_optimizer_dce_mark_statement(Statement* stmt, Declaration** used)
{
	switch (stmt->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		ast_optimizer_dce_mark_expression(stmt->expr_stmt.expression, used);
		break;

	case STATEMENT_TYPE_RETURN:
		ast_optimizer_dce_mark_expression(stmt->return_stmt.expression, used);
		break;

	case STATEMENT_TYPE_DECLARATION:
		// Don't mark declarations' own initializers — only non-decl uses count
		break;

	default:
		break;
	}
}

bool ast_optimizer_dce_is_used(Declaration* decl, Declaration** used)
{
	for (u64 i = 0; i < vector_get_length(used); i++)
	{
		if (used[i] == decl)
			return true;
	}

	return false;
}
