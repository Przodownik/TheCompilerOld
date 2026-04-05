#include "sema.h"

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/diagnostics.h"
#include "wandelt/vector.h"

Sema sema_create(Allocator* decl_allocator, Allocator* expr_allocator, const File* source)
{
	Sema sema           = {0};
	sema.symbol_table   = symtab_create(decl_allocator);
	sema.decl_allocator = decl_allocator;
	sema.expr_allocator = expr_allocator;
	sema.source         = source;
	return sema;
}

bool sema_analyze(Sema* sema, TranslationUnit* tu)
{
	symtab_push_scope(&sema->symbol_table);

	for (AnalysisPass pass = ANALYSIS_PASS_DECLARATIONS; pass < ANALYSIS_PASS_COUNT; pass++)
	{
		if (!sema_analyze_pass(sema, tu, pass))
		{
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}
	}

	symtab_pop_scope(&sema->symbol_table);

	return true;
}

bool sema_analyze_pass(Sema* sema, TranslationUnit* tu, AnalysisPass pass)
{
	static_assert(ANALYSIS_PASS_COUNT == 3, "Update sema_analyze_pass when adding new analysis passes");

	switch (pass)
	{
	case ANALYSIS_PASS_DECLARATIONS:
		return sema_analyze_pass_declarations(sema, tu);

	case ANALYSIS_PASS_DETAILS:
		return sema_analyze_pass_details(sema, tu);

	case ANALYSIS_PASS_UNUSED_VARIABLES:
		return sema_analyze_pass_unused_variables(sema, tu);

	default:
		ASSERT(false, "Unhandled analysis pass in sema_analyze_pass: %d", pass);
		return false;
	}
}

bool sema_analyze_pass_declarations(Sema* sema, TranslationUnit* tu)
{
	(void)sema;

	static_assert(DECLARATION_TYPE_COUNT == 4,
	              "Update sema_pass_register_declarations when adding new declaration types");

	for (u64 i = 0; i < vector_get_length(tu->statements); i++)
	{
		Statement* stmt = tu->statements[i];
		if (stmt->type != STATEMENT_TYPE_DECLARATION)
			continue;

		Declaration* decl = stmt->decl_stmt.declaration;

		switch (decl->type)
		{
		case DECLARATION_TYPE_VARIABLE:
			break; // variables register themselves when encountered during analysis

		case DECLARATION_TYPE_FUNCTION: {
			Symbol* sym = symtab_insert(&sema->symbol_table, decl->fn.name, SYMBOL_KIND_FUNCTION, nullptr, decl);
			if (sym == nullptr)
			{
				diagnostics_verror_along_span(decl->span, sema->source,
				                              "Function '%.*s' was already declared in this namespace",
				                              FMT_STR_ARG(decl->fn.name));
				return false;
			}

			break;
		}

		case DECLARATION_TYPE_NAMESPACE:
			break;

		default:
			ASSERT(false, "Unhandled declaration type in sema_pass_register_declarations: %d", decl->type);
		}
	}

	return true;
}

bool sema_analyze_pass_details(Sema* sema, TranslationUnit* tu)
{
	bool success = true;

	for (u64 i = 0; i < vector_get_length(tu->statements); i++)
	{
		Statement* stmt = tu->statements[i];
		if (!sema_analyze_statement(sema, stmt))
			success = false;
	}

	return success;
}

bool sema_analyze_pass_unused_variables(Sema* sema, TranslationUnit* tu)
{
	(void)tu;
	Scope* scope = sema->symbol_table.current_scope;
	ASSERT(scope);

	Symbol* sym = scope->first_symbol;
	while (sym != nullptr)
	{
		if (sym->kind == SYMBOL_KIND_VARIABLE && !sym->is_used)
		{
			diagnostics_vwarning_along_span(sym->declaration_ref->span, sema->source,
			                                "Variable '%.*s' is declared but never used", FMT_STR_ARG(sym->name));
		}
		sym = sym->next_in_scope;
	}

	return true;
}

bool sema_analyze_statement(Sema* sema, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 9,
	              "sema_analyze_statement needs to be updated to handle new statement types");

	switch (stmt->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		return sema_analyze_declaration_statement(sema, stmt);

	case STATEMENT_TYPE_EXPRESSION:
		return sema_analyze_expression_statement(sema, stmt);

	case STATEMENT_TYPE_RETURN:
		return sema_analyze_return_statement(sema, stmt);

	case STATEMENT_TYPE_ASSIGNMENT:
		return sema_analyze_assignment_statement(sema, stmt);

	case STATEMENT_TYPE_BLOCK:
		return sema_analyze_block_statement(sema, stmt);

	case STATEMENT_TYPE_IF:
		return sema_analyze_if_statement(sema, stmt);

	case STATEMENT_TYPE_FOR:
		return sema_analyze_for_statement(sema, stmt);

	case STATEMENT_TYPE_WHILE:
		return sema_analyze_while_statement(sema, stmt);

	case STATEMENT_TYPE_INVALID:
	case STATEMENT_TYPE_COUNT:
		break;
	}

	ASSERT(false, "Unhandled statement type in sema_analyze_statement: %d", stmt->type);
	return false;
}

bool sema_analyze_return_statement(Sema* sema, Statement* stmt)
{
	if (!sema_check_expression(sema, stmt->return_stmt.expression, nullptr))
		return false;

	return true;
}

bool sema_analyze_assignment_statement(Sema* sema, Statement* stmt)
{
	AssignmentStatement* assign = &stmt->assign_stmt;

	Symbol* sym = symtab_lookup(&sema->symbol_table, assign->target_identifier, true);
	if (sym == nullptr)
	{
		diagnostics_verror_along_span(stmt->span, sema->source, "Undefined variable '%.*s'",
		                              FMT_STR_ARG(assign->target_identifier));
		return false;
	}

	if (sym->kind != SYMBOL_KIND_VARIABLE)
	{
		diagnostics_verror_along_span(stmt->span, sema->source, "'%.*s' is not a variable, so it cannot be assigned to",
		                              FMT_STR_ARG(assign->target_identifier));
		return false;
	}

	ASSERT(sym->declaration_ref->type == DECLARATION_TYPE_VARIABLE);

	sym->declaration_ref->variable.is_ever_assigned = true;

	assign->target_decl_ref = sym->declaration_ref;
	assign->target_type     = sym->type;

	Type* target_type = sym->type;
	if (assign->operator == ASSIGNMENT_OPERATOR_PURE)
	{
		if (!sema_check_expression(sema, assign->value, target_type))
			return false;

		Type* value_type = assign->value->resolved_type;
		if (target_type != value_type)
		{
			if (!type_is_implicitly_convertible(value_type, target_type))
			{
				diagnostics_verror_along_span(
				    stmt->span, sema->source, "Cannot implicitly cast value of type '%s' to variable type '%s'",
				    type_kind_to_cstr(value_type->kind), type_kind_to_cstr(target_type->kind));
				return false;
			}

			assign->value = sema_insert_cast(sema, assign->value, target_type);
		}
	}
	else
	{
		if (!sema_check_expression(sema, assign->value, target_type))
			return false;

		Type* value_type = assign->value->resolved_type;
		if (!type_is_arithmetic(target_type))
		{
			diagnostics_verror_along_span(
			    stmt->span, sema->source, "Cannot use compound assignment '%s' on non-arithmetic type '%s'",
			    assignment_operator_to_token_cstr(assign->operator), type_kind_to_cstr(target_type->kind));
			return false;
		}

		if (value_type != target_type)
		{
			if (assign->value->type == EXPRESSION_TYPE_CONSTANT)
				sema_promote_constant(assign->value, target_type);
			else if (type_is_implicitly_convertible(value_type, target_type))
				assign->value = sema_insert_cast(sema, assign->value, target_type);
			else
			{
				diagnostics_verror_along_span(
				    stmt->span, sema->source, "Incompatible types '%s' and '%s' in compound assignment",
				    type_kind_to_cstr(target_type->kind), type_kind_to_cstr(value_type->kind));
				return false;
			}
		}
	}

	return true;
}

bool sema_analyze_block_statement(Sema* sema, Statement* stmt)
{
	symtab_push_scope(&sema->symbol_table);

	for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
	{
		if (!sema_analyze_statement(sema, stmt->block_stmt.statements[i]))
			return false;
	}

	symtab_pop_scope(&sema->symbol_table);

	return true;
}

bool sema_analyze_if_statement(Sema* sema, Statement* stmt)
{
	if (!sema_check_expression(sema, stmt->if_stmt.condition, nullptr))
		return false;

	Type* cond_type = stmt->if_stmt.condition->resolved_type;
	if (!type_is_bool(cond_type))
	{
		if (type_is_implicitly_convertible(cond_type, type_get_builtin(TYPE_KIND_BOOL)))
		{
			stmt->if_stmt.condition = sema_insert_cast(sema, stmt->if_stmt.condition, type_get_builtin(TYPE_KIND_BOOL));
		}
		else
		{
			diagnostics_verror_along_span(stmt->if_stmt.condition->span, sema->source,
			                              "Condition in 'if' statement must be a boolean expression, got '%s'",
			                              type_kind_to_cstr(cond_type->kind));
			return false;
		}
	}

	if (!sema_analyze_block_statement(sema, stmt->if_stmt.then_block))
		return false;

	if (stmt->if_stmt.else_block)
	{
		if (!sema_analyze_block_statement(sema, stmt->if_stmt.else_block))
			return false;
	}

	return true;
}

static bool sema_is_compile_time_constant_expr(Expression* expr, Declaration* loop_var)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9,
	              "Update sema_is_compile_time_constant_expr when adding new expression types");

	switch (expr->type)
	{
	case EXPRESSION_TYPE_CONSTANT:
		return true;

	case EXPRESSION_TYPE_IDENTIFIER:
		// The loop variable itself is known at unroll time
		if (loop_var && expr->identifier.declaration_ref == loop_var)
			return true;
		// Other identifiers: only if never-assigned constants
		if (expr->identifier.declaration_ref && expr->identifier.declaration_ref->type == DECLARATION_TYPE_VARIABLE &&
		    !expr->identifier.declaration_ref->variable.is_ever_assigned &&
		    expr->identifier.declaration_ref->variable.initializer->type == EXPRESSION_TYPE_CONSTANT)
			return true;
		return false;

	case EXPRESSION_TYPE_BINARY:
		return sema_is_compile_time_constant_expr(expr->binary.left, loop_var) &&
		       sema_is_compile_time_constant_expr(expr->binary.right, loop_var);

	case EXPRESSION_TYPE_UNARY:
		return sema_is_compile_time_constant_expr(expr->unary.operand, loop_var);

	case EXPRESSION_TYPE_GROUP:
		return sema_is_compile_time_constant_expr(expr->group.inner, loop_var);

	case EXPRESSION_TYPE_CAST:
		return sema_is_compile_time_constant_expr(expr->cast.expression, loop_var);

	case EXPRESSION_TYPE_INCDEC:
		// i++ / i-- allowed if operand is the loop variable
		return expr->incdec.operand->type == EXPRESSION_TYPE_IDENTIFIER && loop_var &&
		       expr->incdec.operand->identifier.declaration_ref == loop_var;

	case EXPRESSION_TYPE_CALL:
		return false;

	default:
		return false;
	}
}

bool sema_analyze_for_statement(Sema* sema, Statement* stmt)
{
	symtab_push_scope(&sema->symbol_table);

	if (!sema_analyze_declaration(sema, stmt->for_stmt.initializer))
	{
		symtab_pop_scope(&sema->symbol_table);
		return false;
	}

	if (!sema_check_expression(sema, stmt->for_stmt.condition, nullptr))
	{
		symtab_pop_scope(&sema->symbol_table);
		return false;
	}

	Type* cond_type = stmt->for_stmt.condition->resolved_type;
	if (!type_is_bool(cond_type))
	{
		if (type_is_implicitly_convertible(cond_type, type_get_builtin(TYPE_KIND_BOOL)))
		{
			stmt->for_stmt.condition =
			    sema_insert_cast(sema, stmt->for_stmt.condition, type_get_builtin(TYPE_KIND_BOOL));
		}
		else
		{
			diagnostics_verror_along_span(stmt->for_stmt.condition->span, sema->source,
			                              "For loop condition must be boolean, got '%s'",
			                              type_kind_to_cstr(cond_type->kind));
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}
	}

	if (!sema_check_expression(sema, stmt->for_stmt.update, nullptr))
	{
		symtab_pop_scope(&sema->symbol_table);
		return false;
	}

	if (!sema_analyze_block_statement(sema, stmt->for_stmt.body))
	{
		symtab_pop_scope(&sema->symbol_table);
		return false;
	}

	if (stmt->for_stmt.is_inline)
	{
		Declaration* loop_var = stmt->for_stmt.initializer;
		if (loop_var->variable.initializer->type != EXPRESSION_TYPE_CONSTANT)
		{
			diagnostics_verror_along_span(loop_var->variable.initializer->span, sema->source,
			                              "'inline for' initializer must be a compile-time constant");
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}

		if (!sema_is_compile_time_constant_expr(stmt->for_stmt.condition, loop_var))
		{
			diagnostics_verror_along_span(stmt->for_stmt.condition->span, sema->source,
			                              "'inline for' condition must use compile-time constants");
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}

		if (!sema_is_compile_time_constant_expr(stmt->for_stmt.update, loop_var))
		{
			diagnostics_verror_along_span(stmt->for_stmt.update->span, sema->source,
			                              "'inline for' update must be a compile-time constant step");
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}

		LoopBounds bounds = sema_check_loop_bounds(stmt);
		if (!bounds.is_valid)
		{
			diagnostics_verror_along_span(stmt->span, sema->source,
			                              "Cannot determine iteration count for 'inline for'");
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}

		if (bounds.iteration_count > 1024)
		{
			diagnostics_verror_along_span(stmt->span, sema->source,
			                              "'inline for' iteration count %lld exceeds limit of 1024",
			                              bounds.iteration_count);
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}
	}

	symtab_pop_scope(&sema->symbol_table);

	return true;
}

bool sema_analyze_while_statement(Sema* sema, Statement* stmt)
{
	if (!sema_check_expression(sema, stmt->while_stmt.condition, nullptr))
		return false;

	Type* cond_type = stmt->while_stmt.condition->resolved_type;
	if (!type_is_bool(cond_type))
	{
		if (type_is_implicitly_convertible(cond_type, type_get_builtin(TYPE_KIND_BOOL)))
		{
			stmt->while_stmt.condition =
			    sema_insert_cast(sema, stmt->while_stmt.condition, type_get_builtin(TYPE_KIND_BOOL));
		}
		else
		{
			diagnostics_verror_along_span(stmt->while_stmt.condition->span, sema->source,
			                              "Condition in 'while' must be a boolean expression, got '%s'",
			                              type_kind_to_cstr(cond_type->kind));
			return false;
		}
	}

	if (!sema_analyze_block_statement(sema, stmt->while_stmt.body))
		return false;

	return true;
}

bool sema_analyze_declaration_statement(Sema* sema, Statement* stmt)
{
	return sema_analyze_declaration(sema, stmt->decl_stmt.declaration);
}

bool sema_analyze_declaration(Sema* sema, Declaration* decl)
{
	ASSERT(decl);

	switch (decl->resolve_status)
	{
	case RESOLVE_STATUS_UNRESOLVED:
		decl->resolve_status = RESOLVE_STATUS_RESOLVING;
		bool success         = sema_analyze_declaration_internal(sema, decl);
		decl->resolve_status = RESOLVE_STATUS_RESOLVED;
		if (!success)
		{
			decl->type = DECLARATION_TYPE_INVALID;
		}
		return success;
	case RESOLVE_STATUS_RESOLVING:
		diagnostics_verror_along_span(decl->span, sema->source,
		                              "Cyclic dependency detected while resolving declaration");
		decl->resolve_status = RESOLVE_STATUS_RESOLVED;
		decl->type           = DECLARATION_TYPE_INVALID;
		return false;
	case RESOLVE_STATUS_RESOLVED:
		return decl->type != DECLARATION_TYPE_INVALID;
	}

	ASSERT(false);
}

bool sema_analyze_declaration_internal(Sema* sema, Declaration* decl)
{
	switch (decl->type)
	{
	case DECLARATION_TYPE_NAMESPACE:
		return true;

	case DECLARATION_TYPE_VARIABLE:
		return sema_analyze_variable_declaration(sema, decl);

	case DECLARATION_TYPE_FUNCTION:
		return sema_analyze_function_declaration(sema, decl);

	case DECLARATION_TYPE_INVALID:

	case DECLARATION_TYPE_COUNT:
		break;
	}

	ASSERT(false, "Unhandled declaration type in sema_analyze_declaration_internal: %d", decl->type);
	return false;
}

bool sema_analyze_variable_declaration(Sema* sema, Declaration* decl)
{
	Symbol* sym =
	    symtab_insert(&sema->symbol_table, decl->variable.name, SYMBOL_KIND_VARIABLE, decl->variable.type, decl);

	if (sym == nullptr)
	{
		diagnostics_verror_along_span(decl->span, sema->source, "Variable '%.*s' already declared in this scope",
		                              FMT_STR_ARG(decl->variable.name));
		return false;
	}

	if (!sema_check_expression(sema, decl->variable.initializer, decl->variable.type))
		return false;

	ASSERT(decl->variable.initializer);

	if (decl->variable.type != decl->variable.initializer->resolved_type)
	{
		if (!type_is_implicitly_convertible(decl->variable.initializer->resolved_type, decl->variable.type))
		{
			diagnostics_verror_along_span(decl->span, sema->source,
			                              "Cannot implicitly cast initializer of type '%s' to variable type '%s'",
			                              type_kind_to_cstr(decl->variable.initializer->resolved_type->kind),
			                              type_kind_to_cstr(decl->variable.type->kind));
			return false;
		}

		decl->variable.initializer = sema_insert_cast(sema, decl->variable.initializer, decl->variable.type);
	}

	return true;
}

bool sema_analyze_function_declaration(Sema* sema, Declaration* decl)
{
	symtab_push_scope(&sema->symbol_table);

	FunctionDeclaration* func = &decl->fn;

	for (u64 i = 0; i < vector_get_length(func->parameters); i++)
	{
		FunctionParameter* param = &func->parameters[i];
		if (param->default_value)
		{
			if (!sema_check_expression(sema, param->default_value, param->type))
			{
				symtab_pop_scope(&sema->symbol_table);
				return false;
			}

			Type* default_type = param->default_value->resolved_type;
			if (default_type != param->type)
			{
				if (type_is_implicitly_convertible(default_type, param->type))
				{
					param->default_value = sema_insert_cast(sema, param->default_value, param->type);
				}
				else
				{
					diagnostics_verror_along_span(param->default_value->span, sema->source,
					                              "Default value type '%s' is not compatible with parameter type '%s'",
					                              type_kind_to_cstr(default_type->kind),
					                              type_kind_to_cstr(param->type->kind));
					symtab_pop_scope(&sema->symbol_table);
					return false;
				}
			}
		}

		// probably should be in parser
		Declaration* param_decl          = sema->decl_allocator->alloc(sema->decl_allocator->ctx, sizeof(Declaration));
		param_decl->type                 = DECLARATION_TYPE_VARIABLE;
		param_decl->variable.name        = param->name;
		param_decl->variable.type        = param->type;
		param_decl->variable.initializer = nullptr;
		param_decl->resolve_status       = RESOLVE_STATUS_RESOLVED;
		param_decl->span                 = decl->span;

		Symbol* param_sym =
		    symtab_insert(&sema->symbol_table, param->name, SYMBOL_KIND_VARIABLE, param->type, param_decl);
		if (param_sym == nullptr)
		{
			diagnostics_verror_along_span(param_decl->span, sema->source,
			                              "Parameter with name '%.*s' already declared in this function",
			                              FMT_STR_ARG(param->name));
			symtab_pop_scope(&sema->symbol_table);
			return false;
		}
	}

	bool ok = true;
	for (size_t i = 0; i < vector_get_length(func->body->block_stmt.statements); i++)
	{
		if (!sema_analyze_statement(sema, func->body->block_stmt.statements[i]))
			ok = false;
	}

	symtab_pop_scope(&sema->symbol_table);

	return ok;
}

bool sema_analyze_expression_statement(Sema* sema, Statement* stmt)
{
	return sema_check_expression(sema, stmt->expr_stmt.expression, nullptr);
}

bool sema_check_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	ASSERT(expr);

	switch (expr->resolve_status)
	{
	case RESOLVE_STATUS_UNRESOLVED:
		expr->resolve_status = RESOLVE_STATUS_RESOLVING;
		bool success         = sema_check_expression_internal(sema, expr, type_hint);
		if (success)
		{
			expr->resolve_status = RESOLVE_STATUS_RESOLVED;
		}
		return success;
	case RESOLVE_STATUS_RESOLVING:
		diagnostics_verror_along_span(expr->span, sema->source,
		                              "Cyclic dependency detected while resolving expression");
		return false;

	case RESOLVE_STATUS_RESOLVED:
		return true;
	}

	ASSERT(false);
}

bool sema_check_expression_internal(Sema* sema, Expression* expr, Type* type_hint)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9, "Update sema_check_expression_internal when adding new expression types");

	switch (expr->type)
	{
	case EXPRESSION_TYPE_CONSTANT:
		return sema_check_constant_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_UNARY:
		return sema_check_unary_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_BINARY:
		return sema_check_binary_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_GROUP:
		return sema_check_group_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_IDENTIFIER:
		return sema_check_identifier_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_CAST:
		return sema_check_cast_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_INCDEC:
		return sema_check_incdec_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_CALL:
		return sema_check_call_expression(sema, expr);

	case EXPRESSION_TYPE_INVALID:
	case EXPRESSION_TYPE_COUNT:
		break;
	}

	ASSERT(false, "Unhandled expression type in sema_check_expression_internal: %d", expr->type);
	return false;
}

bool sema_check_constant_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	static_assert(CONSTANT_KIND_COUNT == 5, "Update sema_check_constant_expression when adding new constant kinds");

	(void)sema;

	if (expr->constant.kind == CONSTANT_KIND_INTEGER)
	{
		// If there's a type hint and it's an integer type that can hold this value, use it directly.
		if (type_hint != nullptr && type_is_integer(type_hint) &&
		    expr->constant.integer_value <= ((1ULL << type_hint->size_in_bits) - 1))
		{
			expr->resolved_type = type_hint;
		}
		else
		{
			// No hint: resolve starting from smallest signed type to largest
			if (expr->constant.integer_value <= 0x7F)
				expr->resolved_type = type_get_builtin(TYPE_KIND_CHAR);
			else if (expr->constant.integer_value <= 0x7FFF)
				expr->resolved_type = type_get_builtin(TYPE_KIND_SHORT);
			else if (expr->constant.integer_value <= 0x7FFFFFFF)
				expr->resolved_type = type_get_builtin(TYPE_KIND_INT);
			else
				expr->resolved_type = type_get_builtin(TYPE_KIND_LONG);
		}
	}
	else if (expr->constant.kind == CONSTANT_KIND_FLOAT)
	{
		expr->resolved_type = type_get_builtin(TYPE_KIND_FLOAT);
	}
	else if (expr->constant.kind == CONSTANT_KIND_DOUBLE)
	{
		expr->resolved_type = type_get_builtin(TYPE_KIND_DOUBLE);
	}
	else if (expr->constant.kind == CONSTANT_KIND_BOOLEAN)
	{
		expr->resolved_type = type_get_builtin(TYPE_KIND_BOOL);
	}
	else
	{
		ASSERT(false, "Unhandled constant kind in sema_check_constant_expression: %d", expr->constant.kind);
		return false;
	}

	if (type_hint != nullptr && type_hint != expr->resolved_type)
	{
		Type* common = type_common(type_hint, expr->resolved_type);
		if (common == nullptr)
		{
			diagnostics_verror_along_span(
			    expr->span, sema->source, "Cannot implicitly cast constant of type '%s' to expected type '%s'",
			    type_kind_to_cstr(expr->resolved_type->kind), type_kind_to_cstr(type_hint->kind));
			return false;
		}

		sema_promote_constant(expr, common);
	}

	return true;
}

bool sema_check_unary_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	if (!sema_check_expression(sema, expr->unary.operand, type_hint))
		return false;

	Type* operand_type = expr->unary.operand->resolved_type;

	if (expr->unary.operator == UNARY_OPERATOR_NEGATE)
	{
		bool can_be_negated = type_is_arithmetic(operand_type);

		if (!can_be_negated)
		{
			diagnostics_verror_along_span(expr->span, sema->source, "Cannot negate non-arithmetic type '%s'",
			                              type_kind_to_cstr(operand_type->kind));
			return false;
		}

		// Negating an unsigned type
		if (type_is_unsigned(operand_type))
		{
			diagnostics_verror_along_span(expr->span, sema->source,
			                              "Cannot negate unsigned type '%s', as it would result in an overflow",
			                              type_kind_to_cstr(operand_type->kind));
			return false;
		}
	}

	expr->resolved_type = operand_type;

	return true;
}

bool sema_check_binary_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	// For comparisons, don't propagate the type_hint — the result is bool
	Type* operand_hint = binary_operator_is_comparison(expr->binary.operator) ? nullptr : type_hint;

	if (!sema_check_expression(sema, expr->binary.left, operand_hint))
		return false;

	if (!sema_check_expression(sema, expr->binary.right, operand_hint))
		return false;

	Type* left_type  = expr->binary.left->resolved_type;
	Type* right_type = expr->binary.right->resolved_type;

	Type* common = type_common(left_type, right_type);
	if (common == nullptr)
	{
		diagnostics_verror_along_span(expr->span, sema->source,
		                              "Cannot implicitly cast types '%s' and '%s' in binary expression",
		                              type_kind_to_cstr(left_type->kind), type_kind_to_cstr(right_type->kind));
		return false;
	}

	// Ordering operators (< > <= >=) reject bool operands
	if (binary_operator_is_ordering(expr->binary.operator) && type_is_bool(common))
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Cannot use ordering operator '%s' on 'bool' operands",
		                              binary_operator_to_token_cstr(expr->binary.operator));
		return false;
	}

	if (left_type != common)
	{
		if (expr->binary.left->type == EXPRESSION_TYPE_CONSTANT)
			sema_promote_constant(expr->binary.left, common);
		else
			expr->binary.left = sema_insert_cast(sema, expr->binary.left, common);
	}

	if (right_type != common)
	{
		if (expr->binary.right->type == EXPRESSION_TYPE_CONSTANT)
			sema_promote_constant(expr->binary.right, common);
		else
			expr->binary.right = sema_insert_cast(sema, expr->binary.right, common);
	}

	if (binary_operator_is_comparison(expr->binary.operator))
		expr->resolved_type = type_get_builtin(TYPE_KIND_BOOL);
	else
		expr->resolved_type = common;

	return true;
}

bool sema_check_group_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	if (!sema_check_expression(sema, expr->group.inner, type_hint))
		return false;

	expr->resolved_type = expr->group.inner->resolved_type;

	return true;
}

bool sema_check_identifier_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	(void)type_hint;

	Symbol* sym = symtab_lookup(&sema->symbol_table, expr->identifier.name, true);
	if (sym == nullptr)
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Undefined identifier '%.*s'",
		                              FMT_STR_ARG(expr->identifier.name));
		return false;
	}

	if (sym->declaration_ref->resolve_status == RESOLVE_STATUS_RESOLVING)
	{
		diagnostics_verror_along_span(expr->span, sema->source,
		                              "Cyclic dependency detected while resolving identifier '%.*s'",
		                              FMT_STR_ARG(expr->identifier.name));
		return false;
	}

	if (sym->declaration_ref->type == DECLARATION_TYPE_INVALID)
		return false;

	if (sym->declaration_ref->resolve_status == RESOLVE_STATUS_UNRESOLVED)
	{
		diagnostics_verror_along_span(expr->span, sema->source,
		                              "Cannot resolve identifier '%.*s' because its details have not been resolved yet",
		                              FMT_STR_ARG(expr->identifier.name));
	}

	expr->resolved_type              = sym->type;
	expr->identifier.declaration_ref = sym->declaration_ref;

	return true;
}

bool sema_check_cast_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	(void)type_hint;

	Type* target = expr->cast.target_type;
	ASSERT(target != nullptr && target->kind != TYPE_KIND_INVALID);

	// without hint - the cast itself handles conversion
	if (!sema_check_expression(sema, expr->cast.expression, nullptr))
		return false;

	Type* source = expr->cast.expression->resolved_type;
	if (source == target)
	{
		diagnostics_vwarning_along_span(
		    expr->span, sema->source, "Redundant cast from '%s' to '%s', the type is already '%s'",
		    type_kind_to_cstr(source->kind), type_kind_to_cstr(target->kind), type_kind_to_cstr(source->kind));
		expr->resolved_type = target;
		return true;
	}

	if (type_is_implicitly_convertible(source, target))
	{
		diagnostics_vwarning_along_span(expr->span, sema->source,
		                                "Unnecessary cast: '%s' is implicitly convertible to '%s'",
		                                type_kind_to_cstr(source->kind), type_kind_to_cstr(target->kind));
		expr->resolved_type = target;
		return true;
	}

	if (!type_is_explicitly_castable(source, target))
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Cannot cast from '%s' to '%s'",
		                              type_kind_to_cstr(source->kind), type_kind_to_cstr(target->kind));
		return false;
	}

	expr->resolved_type = target;
	return true;
}

bool sema_check_incdec_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	(void)type_hint;

	// lvalue check
	if (expr->incdec.operand->type != EXPRESSION_TYPE_IDENTIFIER)
	{
		diagnostics_verror_along_span(expr->incdec.operand->span, sema->source, "%s operand must be a variable",
		                              expr->incdec.is_increment ? "Increment" : "Decrement");
		return false;
	}

	if (!sema_check_expression(sema, expr->incdec.operand, nullptr))
		return false;

	Type* operand_type = expr->incdec.operand->resolved_type;
	if (type_is_bool(operand_type))
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Cannot %s 'bool' type",
		                              expr->incdec.is_increment ? "increment" : "decrement");
		return false;
	}

	if (!type_is_arithmetic(operand_type))
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Cannot %s non-arithmetic type '%s'",
		                              expr->incdec.is_increment ? "increment" : "decrement",
		                              type_kind_to_cstr(operand_type->kind));
		return false;
	}

	// Mark variable as "assigned"
	Symbol* sym = symtab_lookup(&sema->symbol_table, expr->incdec.operand->identifier.name, true);
	if (sym->kind != SYMBOL_KIND_VARIABLE)
	{
		diagnostics_verror_along_span(expr->incdec.operand->span, sema->source,
		                              "Cannot apply %s to this expression'%s'",
		                              expr->incdec.is_increment ? "increment" : "decrement");
		return false;
	}

	sym->declaration_ref->variable.is_ever_assigned = true;

	expr->resolved_type = operand_type;

	return true;
}

bool sema_check_call_expression(Sema* sema, Expression* expr)
{
	CallExpression* call = &expr->call;

	Symbol* func_sym = symtab_lookup(&sema->symbol_table, call->function_name, true);
	if (!func_sym)
	{
		diagnostics_verror_along_span(expr->span, sema->source, "Undeclared function '%.*s'",
		                              FMT_STR_ARG(call->function_name));
		return false;
	}

	if (func_sym->kind != SYMBOL_KIND_FUNCTION)
	{
		diagnostics_verror_along_span(expr->span, sema->source, "'%.*s' is not a function",
		                              FMT_STR_ARG(call->function_name));
		return false;
	}

	call->declaration_ref = func_sym->declaration_ref;

	FunctionDeclaration* func_decl = &func_sym->declaration_ref->fn;

	u32 positional_index = 0;
	u64 arg_count        = vector_get_length(call->arguments);
	u64 param_count      = vector_get_length(func_decl->parameters);

	CallArgument* resolved_args = vector_create(sema->expr_allocator, param_count, sizeof(CallArgument));

	for (u64 i = 0; i < arg_count; i++)
	{
		CallArgument* arg = &call->arguments[i];

		if (arg->value == nullptr)
		{
			if (positional_index >= param_count)
			{
				diagnostics_verror_along_span(expr->span, sema->source,
				                              "Too many arguments to function '%.*s' (expected %u but got %u)",
				                              FMT_STR_ARG(call->function_name), param_count, positional_index + 1);
				return false;
			}

			positional_index++;

			continue;
		}

		if (positional_index >= param_count)
		{
			diagnostics_verror_along_span(expr->span, sema->source,
			                              "Too many arguments to function '%.*s' (expected %u but got %u)",
			                              FMT_STR_ARG(call->function_name), param_count, positional_index + 1);
			return false;
		}

		resolved_args[i].value = arg->value;

		positional_index++;
	}

	for (u64 i = 0; i < param_count; i++)
	{
		CallArgument* arg = &resolved_args[i];
		if (arg->value == nullptr)
		{
			if (func_decl->parameters[i].default_value)
			{
				arg->value = func_decl->parameters[i].default_value;
			}
			else
			{
				diagnostics_verror_along_span(
				    expr->span, sema->source,
				    "Missing argument for parameter '%.*s' in call to '%.*s' (no default value)",
				    FMT_STR_ARG(func_decl->parameters[i].name), FMT_STR_ARG(call->function_name));
				return false;
			}
		}
	}

	for (u64 i = 0; i < param_count; i++)
	{
		if (!sema_check_expression(sema, resolved_args[i].value, func_decl->parameters[i].type))
			return false;

		Type* arg_type   = resolved_args[i].value->resolved_type;
		Type* param_type = func_decl->parameters[i].type;

		if (arg_type != param_type)
		{
			if (type_is_implicitly_convertible(arg_type, param_type))
			{
				resolved_args[i].value = sema_insert_cast(sema, resolved_args[i].value, param_type);
			}
			else
			{
				diagnostics_verror_along_span(resolved_args[i].value->span, sema->source,
				                              "Cannot convert argument type '%s' to parameter type '%s' for '%.*s'",
				                              type_kind_to_cstr(arg_type->kind), type_kind_to_cstr(param_type->kind),
				                              FMT_STR_ARG(func_decl->parameters[i].name));
				return false;
			}
		}
	}

	call->arguments     = resolved_args;
	expr->resolved_type = func_decl->return_type;

	return true;
}

void sema_promote_constant(Expression* expr, Type* target)
{
	ASSERT(expr->type == EXPRESSION_TYPE_CONSTANT);

	ConstantKind ck = expr->constant.kind;

	// Extract the source value
	u64 as_u64 = 0;
	f64 as_f64 = 0.0;

	switch (ck)
	{
	case CONSTANT_KIND_BOOLEAN:
		as_u64 = expr->constant.boolean_value ? 1 : 0;
		as_f64 = (f64)as_u64;
		break;
	case CONSTANT_KIND_INTEGER:
		as_u64 = expr->constant.integer_value;
		as_f64 = (f64)as_u64;
		break;
	case CONSTANT_KIND_FLOAT:
		as_f64 = (f64)expr->constant.float_value;
		as_u64 = (u64)as_f64;
		break;
	case CONSTANT_KIND_DOUBLE:
		as_f64 = expr->constant.double_value;
		as_u64 = (u64)as_f64;
		break;
	default:
		break;
	}

	if (type_is_integer(target))
	{
		expr->constant.kind          = CONSTANT_KIND_INTEGER;
		expr->constant.integer_value = as_u64;
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
		expr->constant.boolean_value = as_u64 != 0;
	}

	expr->resolved_type = target;
}

Expression* sema_insert_cast(Sema* sema, Expression* inner, Type* target)
{
	Expression* cast_expr       = sema->expr_allocator->alloc(sema->expr_allocator->ctx, sizeof(Expression));
	cast_expr->type             = EXPRESSION_TYPE_CAST;
	cast_expr->span             = inner->span;
	cast_expr->cast.target_type = target;
	cast_expr->cast.expression  = inner;
	cast_expr->resolved_type    = target;
	cast_expr->resolve_status   = RESOLVE_STATUS_RESOLVED;
	return cast_expr;
}

LoopBounds sema_check_loop_bounds(Statement* stmt)
{
	ASSERT(stmt->type == STATEMENT_TYPE_FOR);

	LoopBounds bounds = {0};

	Declaration* init     = stmt->for_stmt.initializer;
	Expression* init_expr = init->variable.initializer;
	if (init_expr->type != EXPRESSION_TYPE_CONSTANT || init_expr->constant.kind != CONSTANT_KIND_INTEGER)
		return bounds; // for now just constants, in the future maybe folding of some sort will be added

	bounds.start = (i64)init_expr->constant.integer_value;

	Expression* update = stmt->for_stmt.update;
	if (update->type != EXPRESSION_TYPE_INCDEC)
		return bounds;
	bounds.step = update->incdec.is_increment ? 1 : -1;

	Expression* cond = stmt->for_stmt.condition;
	if (cond->type != EXPRESSION_TYPE_BINARY)
		return bounds;

	BinaryOperator op = cond->binary.operator;

	bool left_is_loop_var =
	    cond->binary.left->type == EXPRESSION_TYPE_IDENTIFIER && cond->binary.left->identifier.declaration_ref == init;
	bool right_is_loop_var = cond->binary.right->type == EXPRESSION_TYPE_IDENTIFIER &&
	                         cond->binary.right->identifier.declaration_ref == init;

	Expression* bound_expr;
	if (left_is_loop_var)
	{
		bound_expr = cond->binary.right;
	}
	else if (right_is_loop_var)
	{
		bound_expr = cond->binary.left;

		// Normalize so loop var is always on the left
		if (op == BINARY_OPERATOR_LT)
			op = BINARY_OPERATOR_GT;
		else if (op == BINARY_OPERATOR_GT)
			op = BINARY_OPERATOR_LT;
		else if (op == BINARY_OPERATOR_LEQ)
			op = BINARY_OPERATOR_GEQ;
		else if (op == BINARY_OPERATOR_GEQ)
			op = BINARY_OPERATOR_LEQ;
	}
	else
	{
		return bounds;
	}

	if (bound_expr->type != EXPRESSION_TYPE_CONSTANT || bound_expr->constant.kind != CONSTANT_KIND_INTEGER)
		return bounds;

	bounds.end = (i64)bound_expr->constant.integer_value;
	bounds.op  = op;

	bool ascending = (op == BINARY_OPERATOR_LT || op == BINARY_OPERATOR_LEQ);
	bool inclusive = (op == BINARY_OPERATOR_LEQ || op == BINARY_OPERATOR_GEQ);
	if (!ascending && op != BINARY_OPERATOR_GT && op != BINARY_OPERATOR_GEQ)
		return bounds; // == or != are not safe to unroll

	i64 range = ascending ? (bounds.end - bounds.start) : (bounds.start - bounds.end);
	if (inclusive)
		range++;

	i64 abs_step           = bounds.step > 0 ? bounds.step : -bounds.step;
	bounds.is_valid        = true;
	bounds.iteration_count = range <= 0 ? 0 : (range + abs_step - 1) / abs_step;

	return bounds;
}
