/**
 * @file sema.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast.h"
#include "wandelt/file.h"
#include "wandelt/parser.h"
#include "wandelt/symbol_table.h"

typedef struct Sema
{
	SymbolTable symbol_table;
	Allocator* decl_allocator;
	Allocator* expr_allocator;
	const File* source;
} Sema;

typedef enum AnalysisPass
{
	ANALYSIS_PASS_DECLARATIONS,
	ANALYSIS_PASS_DETAILS,
	ANALYSIS_PASS_UNUSED_VARIABLES,
	ANALYSIS_PASS_COUNT
} AnalysisPass;

Sema sema_create(Allocator* decl_allocator, Allocator* expr_allocator, const File* source);
bool sema_analyze(Sema* sema, TranslationUnit* tu);

bool sema_analyze_pass(Sema* sema, TranslationUnit* tu, AnalysisPass pass);
bool sema_analyze_pass_declarations(Sema* sema, TranslationUnit* tu);
bool sema_analyze_pass_details(Sema* sema, TranslationUnit* tu);
bool sema_analyze_pass_unused_variables(Sema* sema, TranslationUnit* tu);

bool sema_analyze_statement(Sema* sema, Statement* stmt);
bool sema_analyze_return_statement(Sema* sema, Statement* stmt);
bool sema_analyze_assignment_statement(Sema* sema, Statement* stmt);
bool sema_analyze_block_statement(Sema* sema, Statement* stmt);
bool sema_analyze_if_statement(Sema* sema, Statement* stmt);
bool sema_analyze_while_statement(Sema* sema, Statement* stmt);

bool sema_analyze_declaration_statement(Sema* sema, Statement* stmt);
bool sema_analyze_declaration(Sema* sema, Declaration* decl);
bool sema_analyze_declaration_internal(Sema* sema, Declaration* decl);
bool sema_analyze_variable_declaration(Sema* sema, Declaration* decl);

bool sema_analyze_expression_statement(Sema* sema, Statement* stmt);
bool sema_check_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_expression_internal(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_constant_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_unary_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_binary_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_group_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_identifier_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_cast_expression(Sema* sema, Expression* expr, Type* type_hint);
bool sema_check_incdec_expression(Sema* sema, Expression* expr, Type* type_hint);

void sema_promote_constant(Expression* expr, Type* target);
Expression* sema_insert_cast(Sema* sema, Expression* inner, Type* target);
