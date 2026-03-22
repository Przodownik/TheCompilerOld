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
	switch (stmt->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		return sema_analyze_declaration_statement(sema, stmt);
	case STATEMENT_TYPE_EXPRESSION:
		return sema_analyze_expression_statement(sema, stmt);
	case STATEMENT_TYPE_RETURN:
		return sema_analyze_return_statement(sema, stmt);
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
		if (success)
		{
			decl->resolve_status = RESOLVE_STATUS_RESOLVED;
		}
		return success;
	case RESOLVE_STATUS_RESOLVING:
		diagnostics_verror_along_span(decl->span, sema->source,
		                              "Cyclic dependency detected while resolving declaration");
		return false;
	case RESOLVE_STATUS_RESOLVED:
		return true;
	}
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
}

bool sema_check_expression_internal(Sema* sema, Expression* expr, Type* type_hint)
{
	(void)type_hint;

	static_assert(EXPRESSION_TYPE_COUNT == 6, "Update sema_check_expression_internal when adding new expression types");

	switch (expr->type)
	{
	case EXPRESSION_TYPE_CONSTANT:
		return sema_check_constant_expression(sema, expr, type_hint);
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

		expr->resolved_type = common;
	}

	return true;
}

bool sema_check_binary_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	if (!sema_check_expression(sema, expr->binary.left, type_hint))
		return false;

	if (!sema_check_expression(sema, expr->binary.right, type_hint))
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

	if (left_type != common)
	{
		// Constants can just adopt the target type directly — no cast node needed
		if (expr->binary.left->type == EXPRESSION_TYPE_CONSTANT)
			expr->binary.left->resolved_type = common;
		else
			expr->binary.left = sema_insert_cast(sema, expr->binary.left, common);
	}

	if (right_type != common)
	{
		if (expr->binary.right->type == EXPRESSION_TYPE_CONSTANT)
			expr->binary.right->resolved_type = common;
		else
			expr->binary.right = sema_insert_cast(sema, expr->binary.right, common);
	}

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

	expr->resolved_type              = sym->type;
	expr->identifier.declaration_ref = sym->declaration_ref;

	return true;
}

bool sema_check_cast_expression(Sema* sema, Expression* expr, Type* type_hint)
{
	if (!sema_check_expression(sema, expr->cast.expression, type_hint))
		return false;

	expr->resolved_type = expr->cast.target_type;

	return true;
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
