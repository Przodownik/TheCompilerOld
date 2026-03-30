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
	static_assert(DECLARATION_TYPE_COUNT == 3,
	              "Update sema_pass_register_declarations when adding new declaration types");

	bool success = true;

	for (u64 i = 0; i < vector_get_length(tu->statements); i++)
	{
		Statement* stmt = tu->statements[i];
		if (stmt->type != STATEMENT_TYPE_DECLARATION)
			continue;

		Declaration* decl = stmt->decl_stmt.declaration;

		switch (decl->type)
		{
		case DECLARATION_TYPE_VARIABLE: {
			Symbol* sym = symtab_insert(&sema->symbol_table, decl->variable.name, SYMBOL_KIND_VARIABLE,
			                            decl->variable.type, decl);

			if (sym == nullptr)
			{
				diagnostics_verror_along_span(decl->span, sema->source,
				                              "Variable '%.*s' already declared in this scope",
				                              FMT_STR_ARG(decl->variable.name));
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

	return success;
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
	static_assert(STATEMENT_TYPE_COUNT == 7,
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
	case DECLARATION_TYPE_VARIABLE:
		return sema_analyze_variable_declaration(sema, decl);
	case DECLARATION_TYPE_NAMESPACE:
		return true;
	case DECLARATION_TYPE_INVALID:
	case DECLARATION_TYPE_COUNT:
		break;
	}

	ASSERT(false, "Unhandled declaration type in sema_analyze_declaration_internal: %d", decl->type);
	return false;
}

bool sema_analyze_variable_declaration(Sema* sema, Declaration* decl)
{
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
	static_assert(EXPRESSION_TYPE_COUNT == 8, "Update sema_check_expression_internal when adding new expression types");

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
		break;

	case EXPRESSION_TYPE_IDENTIFIER:
		return sema_check_identifier_expression(sema, expr, type_hint);
		break;

	case EXPRESSION_TYPE_CAST:
		return sema_check_cast_expression(sema, expr, type_hint);

	case EXPRESSION_TYPE_INCDEC:
		return sema_check_incdec_expression(sema, expr, type_hint);

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
	ASSERT(sym != nullptr); // for safety, should be handled by check_identifier_expression

	if (sym->declaration_ref->type == DECLARATION_TYPE_VARIABLE)
		sym->declaration_ref->variable.is_ever_assigned = true;

	expr->resolved_type = operand_type;

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
