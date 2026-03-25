#include "ast_opt.h"

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/type.h"
#include "wandelt/vector.h"

AstOptimizer ast_optimizer_create(Allocator* expr_alloc)
{
	AstOptimizer optimizer;
	optimizer.expr_alloc = expr_alloc;
	return optimizer;
}

static bool ast_optimizer_expr_references_decl(Expression* expr, Declaration* decl);

void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu)
{
	// Run propagation and folding per-statement so each declaration is fully
	// optimized before subsequent statements reference it.
	for (size_t i = 0; i < vector_get_length(tu->statements); i++)
	{
		Statement* stmt = tu->statements[i];
		for (AstOptimizationPass pass = 0; pass < AST_OPTIMIZATION_PASS_COUNT; pass++)
		{
			ast_optimizer_optimize_statement(optimizer, pass, stmt);
		}
	}

	// Dead code elimination: remove variable declarations that are no longer
	// referenced by any expression in the program.
	for (size_t i = 0; i < vector_get_length(tu->statements);)
	{
		Statement* stmt = tu->statements[i];
		if (stmt->type != STATEMENT_TYPE_DECLARATION || stmt->decl_stmt.declaration->type != DECLARATION_TYPE_VARIABLE)
		{
			i++;
			continue;
		}

		Declaration* decl = stmt->decl_stmt.declaration;
		bool referenced   = false;

		for (size_t j = 0; j < vector_get_length(tu->statements); j++)
		{
			if (j == i)
				continue;

			Statement* other = tu->statements[j];
			Expression* expr = nullptr;

			switch (other->type)
			{
			case STATEMENT_TYPE_EXPRESSION:
				expr = other->expr_stmt.expression;
				break;
			case STATEMENT_TYPE_RETURN:
				expr = other->return_stmt.expression;
				break;
			case STATEMENT_TYPE_DECLARATION:
				if (other->decl_stmt.declaration->type == DECLARATION_TYPE_VARIABLE)
					expr = other->decl_stmt.declaration->variable.initializer;
				break;
			default:
				break;
			}

			if (expr && ast_optimizer_expr_references_decl(expr, decl))
			{
				referenced = true;
				break;
			}
		}

		if (!referenced)
		{
			vector_remove_at(tu->statements, i, nullptr);
		}
		else
		{
			i++;
		}
	}
}

// Recursively check if any identifier in the expression tree references the given declaration.
static bool ast_optimizer_expr_references_decl(Expression* expr, Declaration* decl)
{
	static_assert(EXPRESSION_TYPE_COUNT == 7, "Update this function when adding new expression types");

	switch (expr->type)
	{
	case EXPRESSION_TYPE_IDENTIFIER:
		return expr->identifier.declaration_ref == decl;
	case EXPRESSION_TYPE_UNARY:
		return ast_optimizer_expr_references_decl(expr->unary.operand, decl);
	case EXPRESSION_TYPE_BINARY:
		return ast_optimizer_expr_references_decl(expr->binary.left, decl) ||
		       ast_optimizer_expr_references_decl(expr->binary.right, decl);
	case EXPRESSION_TYPE_GROUP:
		return ast_optimizer_expr_references_decl(expr->group.inner, decl);
	case EXPRESSION_TYPE_CAST:
		return ast_optimizer_expr_references_decl(expr->cast.expression, decl);
	default:
		return false;
	}
}

void ast_optimizer_optimize_statement(AstOptimizer* optimizer, AstOptimizationPass pass, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update this function when adding new statement types");
	ASSERT(stmt->type != STATEMENT_TYPE_INVALID);

	switch (stmt->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		stmt->expr_stmt.expression = ast_optimizer_optimize_expression(optimizer, pass, stmt->expr_stmt.expression);
		break;
	case STATEMENT_TYPE_RETURN:
		stmt->return_stmt.expression = ast_optimizer_optimize_expression(optimizer, pass, stmt->return_stmt.expression);
		break;
	case STATEMENT_TYPE_DECLARATION: {
		Declaration* decl = stmt->decl_stmt.declaration;
		if (decl->type == DECLARATION_TYPE_VARIABLE && decl->variable.initializer)
		{
			decl->variable.initializer = ast_optimizer_optimize_expression(optimizer, pass, decl->variable.initializer);
		}
		break;
	}
	case STATEMENT_TYPE_COUNT:
	default:
		break;
	}
}

Expression* ast_optimizer_optimize_expression(AstOptimizer* optimizer, AstOptimizationPass pass, Expression* expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 7, "Update this function when adding new expression types");
	ASSERT(expr->type != EXPRESSION_TYPE_INVALID);

	switch (expr->type)
	{
	case EXPRESSION_TYPE_UNARY:
		expr->unary.operand = ast_optimizer_optimize_expression(optimizer, pass, expr->unary.operand);
		break;

	case EXPRESSION_TYPE_BINARY:
		expr->binary.left  = ast_optimizer_optimize_expression(optimizer, pass, expr->binary.left);
		expr->binary.right = ast_optimizer_optimize_expression(optimizer, pass, expr->binary.right);
		break;

	case EXPRESSION_TYPE_GROUP:
		return ast_optimizer_optimize_expression(optimizer, pass, expr->group.inner);

	case EXPRESSION_TYPE_CONSTANT:
	case EXPRESSION_TYPE_COUNT:
	default:
		break;

	case EXPRESSION_TYPE_IDENTIFIER:
		if (pass == AST_OPTIMIZATION_PASS_CONSTANT_PROPAGATION)
		{
			// If this identifier refers to a variable whose initializer has already
			// been folded down to an integer constant, replace the identifier with that constant.
			Declaration* decl = expr->identifier.declaration_ref;
			if (decl && decl->type == DECLARATION_TYPE_VARIABLE && decl->variable.initializer &&
			    decl->variable.initializer->type == EXPRESSION_TYPE_CONSTANT)
			{
				expr->type     = EXPRESSION_TYPE_CONSTANT;
				expr->constant = decl->variable.initializer->constant;
			}
		}
		break;

	case EXPRESSION_TYPE_CAST:
		expr->cast.expression = ast_optimizer_optimize_expression(optimizer, pass, expr->cast.expression);
		break;
	}

	switch (pass)
	{
	case AST_OPTIMIZATION_PASS_CONSTANT_PROPAGATION:
		return expr;

	case AST_OPTIMIZATION_PASS_CONSTANT_FOLDING:
		return ast_optimizer_constant_fold_expression_pass(optimizer, expr);

	default:
		return expr;
	}
}

static Expression* ast_optimizer_fold_cast(Expression* expr)
{
	ASSERT(expr->type == EXPRESSION_TYPE_CAST);

	Expression* inner = expr->cast.expression;
	if (inner->type != EXPRESSION_TYPE_CONSTANT)
		return expr;

	Type* target    = expr->cast.target_type;
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

	expr->type = EXPRESSION_TYPE_CONSTANT;

	if (type_is_integer(target))
	{
		expr->constant.kind          = CONSTANT_KIND_INTEGER;
		expr->constant.integer_value = (u64)as_i64;
	}
	else if (target->kind == TYPE_KIND_FLOAT)
	{
		expr->constant.kind        = CONSTANT_KIND_FLOAT;
		expr->constant.float_value = (float)as_f64;
	}
	else if (target->kind == TYPE_KIND_DOUBLE)
	{
		expr->constant.kind         = CONSTANT_KIND_DOUBLE;
		expr->constant.double_value = as_f64;
	}
	else if (target->kind == TYPE_KIND_BOOL)
	{
		expr->constant.kind          = CONSTANT_KIND_BOOLEAN;
		expr->constant.boolean_value = as_i64 != 0;
	}
	else
	{
		return expr; // unknown target, leave as-is
	}

	expr->resolved_type = target;
	return expr;
}

static Expression* ast_optimizer_fold_unary(Expression* expr)
{
	ASSERT(expr->type == EXPRESSION_TYPE_UNARY);
	Expression* operand = expr->unary.operand;
	if (operand->type != EXPRESSION_TYPE_CONSTANT)
		return expr;

	if (expr->unary.operator != UNARY_OPERATOR_NEGATE)
		return expr;

	switch (operand->constant.kind)
	{
	case CONSTANT_KIND_INTEGER:
		expr->type                   = EXPRESSION_TYPE_CONSTANT;
		expr->constant.kind          = CONSTANT_KIND_INTEGER;
		expr->constant.integer_value = (u64)(-(i64)operand->constant.integer_value);
		return expr;
	case CONSTANT_KIND_FLOAT:
		expr->type                 = EXPRESSION_TYPE_CONSTANT;
		expr->constant.kind        = CONSTANT_KIND_FLOAT;
		expr->constant.float_value = -operand->constant.float_value;
		return expr;
	case CONSTANT_KIND_DOUBLE:
		expr->type                  = EXPRESSION_TYPE_CONSTANT;
		expr->constant.kind         = CONSTANT_KIND_DOUBLE;
		expr->constant.double_value = -operand->constant.double_value;
		return expr;
	default:
		return expr;
	}
}

static Expression* ast_optimizer_fold_integer_binary(Expression* expr, i64 lval, i64 rval)
{
	i64 result;
	switch (expr->binary.operator)
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
		return expr;
	}
	expr->type                   = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind          = CONSTANT_KIND_INTEGER;
	expr->constant.integer_value = (u64)result;
	return expr;
}

static Expression* ast_optimizer_fold_float_binary(Expression* expr, float lval, float rval)
{
	float result;
	switch (expr->binary.operator)
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
		return expr;
	}
	expr->type                 = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind        = CONSTANT_KIND_FLOAT;
	expr->constant.float_value = result;
	return expr;
}

static Expression* ast_optimizer_fold_double_binary(Expression* expr, double lval, double rval)
{
	double result;
	switch (expr->binary.operator)
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
		return expr;
	}
	expr->type                  = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind         = CONSTANT_KIND_DOUBLE;
	expr->constant.double_value = result;
	return expr;
}

Expression* ast_optimizer_constant_fold_expression_pass(AstOptimizer* optimizer, Expression* expr)
{
	(void)optimizer;
	static_assert(BINARY_OPERATOR_COUNT == 5, "Update this function when adding new binary operators");

	if (expr->type == EXPRESSION_TYPE_UNARY)
		return ast_optimizer_fold_unary(expr);

	if (expr->type == EXPRESSION_TYPE_CAST)
		return ast_optimizer_fold_cast(expr);

	if (expr->type != EXPRESSION_TYPE_BINARY)
		return expr;

	Expression* left  = expr->binary.left;
	Expression* right = expr->binary.right;

	if (left->type != EXPRESSION_TYPE_CONSTANT || right->type != EXPRESSION_TYPE_CONSTANT)
		return expr;

	// Both operands must be the same constant kind (sema ensures type promotion)
	if (left->constant.kind != right->constant.kind)
		return expr;
	switch (left->constant.kind)
	{
	case CONSTANT_KIND_INTEGER:
		return ast_optimizer_fold_integer_binary(expr, (i64)left->constant.integer_value,
		                                         (i64)right->constant.integer_value);
	case CONSTANT_KIND_FLOAT:
		return ast_optimizer_fold_float_binary(expr, left->constant.float_value, right->constant.float_value);
	case CONSTANT_KIND_DOUBLE:
		return ast_optimizer_fold_double_binary(expr, left->constant.double_value, right->constant.double_value);
	default:
		return expr;
	}
}
