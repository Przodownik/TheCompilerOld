#include "ast_opt.h"

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/sema.h"
#include "wandelt/type.h"
#include "wandelt/vector.h"

AstOptimizer ast_optimizer_create(Allocator* stmt_alloc, Allocator* decl_alloc, Allocator* expr_alloc)
{
	return (AstOptimizer){
	    .stmt_alloc = stmt_alloc,
	    .decl_alloc = decl_alloc,
	    .expr_alloc = expr_alloc,
	};
}

void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu, bool optimize)
{
	ast_optimizer_unroll_inline_for_statements(optimizer, tu->statements);

	if (!optimize)
		return;

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

static bool inline_for_eval_condition(i64 iter, BinaryOperator op, i64 end)
{
	switch (op)
	{
	case BINARY_OPERATOR_LT:
		return iter < end;
	case BINARY_OPERATOR_LEQ:
		return iter <= end;
	case BINARY_OPERATOR_GT:
		return iter > end;
	case BINARY_OPERATOR_GEQ:
		return iter >= end;
	default:
		return false;
	}
}

void ast_optimizer_unroll_inline_for_statements(AstOptimizer* optimizer, Statement** stmts)
{
	for (u64 i = 0; i < vector_get_length(stmts); i++)
	{
		Statement* stmt = stmts[i];

		static_assert(STATEMENT_TYPE_COUNT == 9,
		              "ast_optimizer_unroll_inline_for_statements needs to be updated to handle new statement types");

		switch (stmt->type)
		{
		case STATEMENT_TYPE_BLOCK:
			ast_optimizer_unroll_inline_for_statements(optimizer, stmt->block_stmt.statements);
			continue;
		case STATEMENT_TYPE_IF:
			ast_optimizer_unroll_inline_for_statements(optimizer, stmt->if_stmt.then_block->block_stmt.statements);
			if (stmt->if_stmt.else_block)
				ast_optimizer_unroll_inline_for_statements(optimizer, stmt->if_stmt.else_block->block_stmt.statements);
			continue;
		case STATEMENT_TYPE_WHILE:
			ast_optimizer_unroll_inline_for_statements(optimizer, stmt->while_stmt.body->block_stmt.statements);
			continue;
		case STATEMENT_TYPE_FOR:
			if (!stmt->for_stmt.is_inline)
			{
				ast_optimizer_unroll_inline_for_statements(optimizer, stmt->for_stmt.body->block_stmt.statements);
				continue;
			}
			break;
		default:
			continue;
		}

		// Unroll inline for
		LoopBounds bounds = sema_check_loop_bounds(stmt);
		ASSERT(bounds.is_valid, "Inline for bounds must be valid (sema should have caught this)");

		Declaration* loop_var = stmt->for_stmt.initializer;
		Type* loop_type       = loop_var->variable.type;
		Statement* body       = stmt->for_stmt.body;
		u64 body_len          = vector_get_length(body->block_stmt.statements);

		AstCopyContext ctx = {
		    .stmt_alloc  = optimizer->stmt_alloc,
		    .expr_alloc  = optimizer->expr_alloc,
		    .decl_alloc  = optimizer->decl_alloc,
		    .subst_decl  = loop_var,
		    .subst_type  = loop_type,
		    .subst_value = 0,
		};

		Statement* outer_block = optimizer->stmt_alloc->alloc(optimizer->stmt_alloc->ctx, sizeof(Statement));
		outer_block->block_stmt.statements = vector_create(optimizer->stmt_alloc, 4, sizeof(Statement*));
		outer_block->type                  = STATEMENT_TYPE_BLOCK;
		outer_block->span                  = stmt->span;
		outer_block->next                  = stmt->next;

		for (i64 iter = bounds.start; inline_for_eval_condition(iter, bounds.op, bounds.end); iter += bounds.step)
		{
			Statement* inner_block = optimizer->stmt_alloc->alloc(optimizer->stmt_alloc->ctx, sizeof(Statement));
			inner_block->block_stmt.statements = vector_create(optimizer->stmt_alloc, 4, sizeof(Statement*));
			inner_block->type                  = STATEMENT_TYPE_BLOCK;
			inner_block->span                  = stmt->span;
			inner_block->next                  = stmt->next;

			ctx.subst_value = iter;

			for (u64 j = 0; j < body_len; j++)
			{
				Statement* original = body->block_stmt.statements[j];
				Statement* copy     = ast_deep_copy_statement(&ctx, original);

				vector_push(inner_block->block_stmt.statements, copy);
			}

			vector_push(outer_block->block_stmt.statements, inner_block);
		}

		stmts[i] = outer_block;

		// handle nested inline fors
		ast_optimizer_unroll_inline_for_statements(optimizer, outer_block->block_stmt.statements);
	}
}

bool ast_optimizer_fold_statement(AstOptimizer* optimizer, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 9, "Update this function when adding new statement types");

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

	case STATEMENT_TYPE_BLOCK:
		return ast_optimizer_fold_block_statement(optimizer, stmt);

	case STATEMENT_TYPE_IF:
		return ast_optimizer_fold_if_statement(optimizer, stmt);

	case STATEMENT_TYPE_FOR:
		return ast_optimizer_fold_for_statement(optimizer, stmt);

	case STATEMENT_TYPE_WHILE:
		return ast_optimizer_fold_while_statement(optimizer, stmt);

	case STATEMENT_TYPE_ASSIGNMENT:
		return ast_optimizer_fold_assignment_statement(optimizer, stmt);

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

bool ast_optimizer_fold_block_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
		changed |= ast_optimizer_fold_statement(optimizer, stmt->block_stmt.statements[i]);

	return changed;
}

bool ast_optimizer_fold_if_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_fold_expression(optimizer, &stmt->if_stmt.condition);
	changed |= ast_optimizer_fold_statement(optimizer, stmt->if_stmt.then_block);

	if (stmt->if_stmt.else_block)
		changed |= ast_optimizer_fold_statement(optimizer, stmt->if_stmt.else_block);

	return changed;
}

bool ast_optimizer_fold_for_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_fold_expression(optimizer, &stmt->for_stmt.initializer->variable.initializer);
	changed |= ast_optimizer_fold_expression(optimizer, &stmt->for_stmt.condition);
	changed |= ast_optimizer_fold_expression(optimizer, &stmt->for_stmt.update);
	changed |= ast_optimizer_fold_statement(optimizer, stmt->for_stmt.body);

	return changed;
}

bool ast_optimizer_fold_while_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_fold_expression(optimizer, &stmt->while_stmt.condition);
	changed |= ast_optimizer_fold_statement(optimizer, stmt->while_stmt.body);

	return changed;
}

bool ast_optimizer_fold_assignment_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_fold_expression(optimizer, &stmt->assign_stmt.value);
}

bool ast_optimizer_fold_expression(AstOptimizer* optimizer, Expression** expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9, "Update this function when adding new expression types");

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

	case EXPRESSION_TYPE_INCDEC:
		return ast_optimizer_fold_incdec_expression(optimizer, expr);

	case EXPRESSION_TYPE_CALL: {
		bool changed = false;
		for (u64 i = 0; i < vector_get_length((*expr)->call.arguments); i++)
		{
			CallArgument arg = (*expr)->call.arguments[i];
			if (arg.value)
				changed |= ast_optimizer_fold_incdec_expression(optimizer, &arg.value);
		}

		return changed;
	}

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

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->unary.operand);

	if (expression->unary.operand->type != EXPRESSION_TYPE_CONSTANT)
		return changed;

	if (expression->unary.operator == UNARY_OPERATOR_NEGATE)
	{
		switch (expression->unary.operand->constant.kind)
		{
		case CONSTANT_KIND_INVALID:
			ASSERT(false, "Invalid constant.");
			break;

		case CONSTANT_KIND_INTEGER:
			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_INTEGER;
			expression->constant.integer_value = (u64)(-(i64)expression->unary.operand->constant.integer_value);
			return true;

		case CONSTANT_KIND_FLOAT:
			expression->type                 = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind        = CONSTANT_KIND_FLOAT;
			expression->constant.float_value = -expression->unary.operand->constant.float_value;
			return true;

		case CONSTANT_KIND_DOUBLE:
			expression->type                  = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind         = CONSTANT_KIND_DOUBLE;
			expression->constant.double_value = -expression->unary.operand->constant.double_value;
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

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->binary.left);
	changed |= ast_optimizer_fold_expression(optimizer, &expression->binary.right);

	if (expression->binary.left->type != EXPRESSION_TYPE_CONSTANT ||
	    expression->binary.right->type != EXPRESSION_TYPE_CONSTANT)
		return changed;

	ASSERT(expression->binary.left->constant.kind == expression->binary.right->constant.kind);

	switch (expression->binary.left->constant.kind)
	{
	case CONSTANT_KIND_INVALID:
		ASSERT(false, "Invalid constant.");
		break;

	case CONSTANT_KIND_INTEGER: {
		u64 lval = expression->binary.left->constant.integer_value;
		u64 rval = expression->binary.right->constant.integer_value;

		if (binary_operator_is_comparison(expression->binary.operator))
		{
			bool result = false;

			if (type_is_signed(expression->binary.left->resolved_type))
			{
				i64 slval = (i64)lval;
				i64 srval = (i64)rval;

				switch (expression->binary.operator)
				{
				case BINARY_OPERATOR_EQ:
					result = slval == srval;
					break;
				case BINARY_OPERATOR_NEQ:
					result = slval != srval;
					break;
				case BINARY_OPERATOR_LT:
					result = slval < srval;
					break;
				case BINARY_OPERATOR_GT:
					result = slval > srval;
					break;
				case BINARY_OPERATOR_LEQ:
					result = slval <= srval;
					break;
				case BINARY_OPERATOR_GEQ:
					result = slval >= srval;
					break;
				default:
					ASSERT(false, "Not a comparison operator");
					break;
				}
			}
			else
			{
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
			}

			expression->type                   = EXPRESSION_TYPE_CONSTANT;
			expression->constant.kind          = CONSTANT_KIND_BOOLEAN;
			expression->constant.boolean_value = result;

			return true;
		}

		u64 result;

		if (type_is_signed(expression->binary.left->resolved_type))
		{
			i64 slval = (i64)lval;
			i64 srval = (i64)rval;

			switch (expression->binary.operator)
			{
			case BINARY_OPERATOR_ADD:
				result = (u64)(slval + srval);
				break;
			case BINARY_OPERATOR_SUB:
				result = (u64)(slval - srval);
				break;
			case BINARY_OPERATOR_MUL:
				result = (u64)(slval * srval);
				break;
			case BINARY_OPERATOR_DIV:
				ASSERT(srval != 0);
				result = (u64)(slval / srval);
				break;
			default:
				ASSERT(false, "Not a supported binary operator for integer constant folding.");
				break;
			}
		}
		else
		{
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
		}

		expression->type                   = EXPRESSION_TYPE_CONSTANT;
		expression->constant.kind          = CONSTANT_KIND_INTEGER;
		expression->constant.integer_value = (u64)result;

		return true;
	}

	case CONSTANT_KIND_FLOAT: {
		float lval = expression->binary.left->constant.float_value;
		float rval = expression->binary.right->constant.float_value;

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
		double lval = expression->binary.left->constant.double_value;
		double rval = expression->binary.right->constant.double_value;

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

	bool changed = ast_optimizer_fold_expression(optimizer, &expression->cast.expression);

	if (expression->cast.expression->type == EXPRESSION_TYPE_CONSTANT)
	{
		Type* target    = expression->cast.target_type;
		ConstantKind ck = expression->cast.expression->constant.kind;

		i64 as_i64 = 0;
		f64 as_f64 = 0.0;

		switch (ck)
		{
		case CONSTANT_KIND_BOOLEAN:
			as_i64 = expression->cast.expression->constant.boolean_value ? 1 : 0;
			as_f64 = (f64)as_i64;
			break;
		case CONSTANT_KIND_INTEGER:
			as_i64 = (i64)expression->cast.expression->constant.integer_value;
			as_f64 = (f64)as_i64;
			break;
		case CONSTANT_KIND_FLOAT:
			as_f64 = (f64)expression->cast.expression->constant.float_value;
			as_i64 = (i64)as_f64;
			break;
		case CONSTANT_KIND_DOUBLE:
			as_f64 = expression->cast.expression->constant.double_value;
			as_i64 = (i64)as_f64;
			break;
		default:
			ASSERT(false, "Invalid constant.");
			break;
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

bool ast_optimizer_fold_incdec_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;
	(void)expr;

	// Cannot fold has a side effect, data flow analysis needed
	return false;
}

bool ast_optimizer_propagate_statement(AstOptimizer* optimizer, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 9, "Update this function when adding new statement types");

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

	case STATEMENT_TYPE_BLOCK:
		return ast_optimizer_propagate_block_statement(optimizer, stmt);

	case STATEMENT_TYPE_IF:
		return ast_optimizer_propagate_if_statement(optimizer, stmt);

	case STATEMENT_TYPE_FOR:
		return ast_optimizer_propagate_for_statement(optimizer, stmt);

	case STATEMENT_TYPE_WHILE:
		return ast_optimizer_propagate_while_statement(optimizer, stmt);

	case STATEMENT_TYPE_ASSIGNMENT:
		return ast_optimizer_propagate_assignment_statement(optimizer, stmt);

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

bool ast_optimizer_propagate_block_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
		changed |= ast_optimizer_propagate_statement(optimizer, stmt->block_stmt.statements[i]);

	return changed;
}

bool ast_optimizer_propagate_if_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_propagate_expression(optimizer, &stmt->if_stmt.condition);
	changed |= ast_optimizer_propagate_statement(optimizer, stmt->if_stmt.then_block);

	if (stmt->if_stmt.else_block)
		changed |= ast_optimizer_propagate_statement(optimizer, stmt->if_stmt.else_block);

	return changed;
}

bool ast_optimizer_propagate_for_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_propagate_expression(optimizer, &stmt->for_stmt.initializer->variable.initializer);
	changed |= ast_optimizer_propagate_expression(optimizer, &stmt->for_stmt.condition);
	changed |= ast_optimizer_propagate_expression(optimizer, &stmt->for_stmt.update);
	changed |= ast_optimizer_propagate_statement(optimizer, stmt->for_stmt.body);

	return changed;
}

bool ast_optimizer_propagate_while_statement(AstOptimizer* optimizer, Statement* stmt)
{
	bool changed = false;

	changed |= ast_optimizer_propagate_expression(optimizer, &stmt->while_stmt.condition);
	changed |= ast_optimizer_propagate_statement(optimizer, stmt->while_stmt.body);

	return changed;
}

bool ast_optimizer_propagate_assignment_statement(AstOptimizer* optimizer, Statement* stmt)
{
	return ast_optimizer_propagate_expression(optimizer, &stmt->assign_stmt.value);
}

bool ast_optimizer_propagate_expression(AstOptimizer* optimizer, Expression** expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9, "Update this function when adding new expression types");

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

	case EXPRESSION_TYPE_INCDEC:
		return ast_optimizer_propagate_incdec_expression(optimizer, expr);

	case EXPRESSION_TYPE_CALL: {
		bool changed = false;
		for (u64 i = 0; i < vector_get_length((*expr)->call.arguments); i++)
		{
			CallArgument arg = (*expr)->call.arguments[i];
			if (arg.value)
				changed |= ast_optimizer_propagate_incdec_expression(optimizer, &arg.value);
		}

		return changed;
	}

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

	// Do not propagate if the variable has been assigned to
	if (decl->variable.is_ever_assigned)
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

bool ast_optimizer_propagate_incdec_expression(AstOptimizer* optimizer, Expression** expr)
{
	(void)optimizer;
	(void)expr;

	// Cannot fold has a side effect, data flow analysis needed
	return false;
}

void ast_optimizer_dce(AstOptimizer* optimizer, TranslationUnit* tu)
{
	(void)optimizer;

	Allocator heap     = allocator_get_heap_allocator();
	Declaration** used = vector_create(&heap, 16, sizeof(Declaration*));

	for (u64 i = 0; i < vector_get_length(tu->statements); i++)
		ast_optimizer_dce_mark_statement(tu->statements[i], &used);

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

void ast_optimizer_dce_mark_expression(Expression* expr, Declaration*** used)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9, "Update this function when adding new expression types");

	switch (expr->type)
	{
	case EXPRESSION_TYPE_IDENTIFIER:
		if (expr->identifier.declaration_ref)
			vector_push(*used, expr->identifier.declaration_ref);
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

	case EXPRESSION_TYPE_INCDEC:
		ast_optimizer_dce_mark_expression(expr->incdec.operand, used);
		break;

	case EXPRESSION_TYPE_CALL:
		for (u64 i = 0; i < vector_get_length(expr->call.arguments); i++)
		{
			CallArgument arg = expr->call.arguments[i];
			if (arg.value)
				ast_optimizer_dce_mark_expression(arg.value, used);
		}

		break;

	case EXPRESSION_TYPE_CONSTANT:
	default:
		break;
	}
}

void ast_optimizer_dce_mark_statement(Statement* stmt, Declaration*** used)
{
	static_assert(STATEMENT_TYPE_COUNT == 9, "Update this function when adding new statement types");

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

	case STATEMENT_TYPE_BLOCK:
		for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
			ast_optimizer_dce_mark_statement(stmt->block_stmt.statements[i], used);
		break;

	case STATEMENT_TYPE_IF:
		ast_optimizer_dce_mark_expression(stmt->if_stmt.condition, used);
		ast_optimizer_dce_mark_statement(stmt->if_stmt.then_block, used);
		if (stmt->if_stmt.else_block)
			ast_optimizer_dce_mark_statement(stmt->if_stmt.else_block, used);
		break;

	case STATEMENT_TYPE_FOR:
		ast_optimizer_dce_mark_expression(stmt->for_stmt.condition, used);
		ast_optimizer_dce_mark_expression(stmt->for_stmt.update, used);
		ast_optimizer_dce_mark_statement(stmt->for_stmt.body, used);
		break;

	case STATEMENT_TYPE_WHILE:
		ast_optimizer_dce_mark_expression(stmt->while_stmt.condition, used);
		ast_optimizer_dce_mark_statement(stmt->while_stmt.body, used);
		break;

	case STATEMENT_TYPE_ASSIGNMENT:
		if (stmt->assign_stmt.target_decl_ref)
			vector_push(*used, stmt->assign_stmt.target_decl_ref);

		ast_optimizer_dce_mark_expression(stmt->assign_stmt.value, used);
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
