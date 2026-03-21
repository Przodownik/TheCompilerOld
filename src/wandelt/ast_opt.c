#include "ast_opt.h"

#include "defines.h"
#include "wandelt/ast.h"
#include "wandelt/vector.h"

AstOptimizer ast_optimizer_create(Allocator* expr_alloc)
{
	AstOptimizer optimizer;
	optimizer.expr_alloc = expr_alloc;
	return optimizer;
}

void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu)
{
	for (AstOptimizationPass pass = AST_OPTIMIZATION_PASS_CONSTANT_FOLDING; pass < AST_OPTIMIZATION_PASS_COUNT; pass++)
	{
		ast_optimizer_run_pass(optimizer, pass, tu);
	}
}

void ast_optimizer_run_pass(AstOptimizer* optimizer, AstOptimizationPass pass, TranslationUnit* tu)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update this function when adding new statement types");

	for (size_t i = 0; i < vector_get_length(tu->statements); i++)
	{
		Statement* stmt = tu->statements[i];
		ASSERT(stmt->type != STATEMENT_TYPE_INVALID);

		switch (stmt->type)
		{
		case STATEMENT_TYPE_EXPRESSION:
			stmt->expr_stmt.expression = ast_optimizer_optimize_expression(optimizer, pass, stmt->expr_stmt.expression);
			break;
		case STATEMENT_TYPE_RETURN:
			stmt->return_stmt.expression =
			    ast_optimizer_optimize_expression(optimizer, pass, stmt->return_stmt.expression);
			break;
		case STATEMENT_TYPE_DECLARATION:
		case STATEMENT_TYPE_COUNT:
		default:
			break;
		}
	}
}

Expression* ast_optimizer_optimize_expression(AstOptimizer* optimizer, AstOptimizationPass pass, Expression* expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 4, "Update this function when adding new expression types");
	ASSERT(expr->type != EXPRESSION_TYPE_INVALID);

	switch (expr->type)
	{
	case EXPRESSION_TYPE_BINARY:
		expr->binary.left  = ast_optimizer_optimize_expression(optimizer, pass, expr->binary.left);
		expr->binary.right = ast_optimizer_optimize_expression(optimizer, pass, expr->binary.right);
		break;

	case EXPRESSION_TYPE_CONSTANT:
	case EXPRESSION_TYPE_IDENTIFIER:
	case EXPRESSION_TYPE_COUNT:
	default:
		break;
	}

	switch (pass)
	{
	case AST_OPTIMIZATION_PASS_CONSTANT_FOLDING:
		return ast_optimizer_constant_fold_expression_pass(optimizer, expr);

	default:
		return expr;
	}
}

Expression* ast_optimizer_constant_fold_expression_pass(AstOptimizer* optimizer, Expression* expr)
{
	(void)optimizer;
	static_assert(BINARY_OPERATOR_COUNT == 5, "Update this function when adding new binary operators");

	if (expr->type != EXPRESSION_TYPE_BINARY)
		return expr;

	Expression* left  = expr->binary.left;
	Expression* right = expr->binary.right;

	if (left->type != EXPRESSION_TYPE_CONSTANT || right->type != EXPRESSION_TYPE_CONSTANT)
		return expr;

	// temp
	if (left->constant.kind != CONSTANT_KIND_INTEGER || right->constant.kind != CONSTANT_KIND_INTEGER)
		return expr;

	u64 lval = left->constant.integer;
	u64 rval = right->constant.integer;
	u64 result;

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

	// Rewrite this node in-place as a constant
	expr->type             = EXPRESSION_TYPE_CONSTANT;
	expr->constant.kind    = CONSTANT_KIND_INTEGER;
	expr->constant.integer = result;

	return expr;
}
