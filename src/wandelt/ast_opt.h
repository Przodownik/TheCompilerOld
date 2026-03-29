/**
 * @file ast_opt.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast.h"
#include "wandelt/memory.h"
#include "wandelt/parser.h"

typedef struct AstOptimizer
{
	Allocator* expr_alloc;
} AstOptimizer;

AstOptimizer ast_optimizer_create(Allocator* expr_alloc);
void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu);

bool ast_optimizer_fold_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_fold_declaration_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_fold_expression_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_fold_return_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_fold_assignment_statement(AstOptimizer* optimizer, Statement* stmt);

bool ast_optimizer_fold_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_constant_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_unary_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_binary_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_group_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_identifier_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_cast_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_fold_incdec_expression(AstOptimizer* optimizer, Expression** expr);

bool ast_optimizer_propagate_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_propagate_declaration_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_propagate_expression_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_propagate_return_statement(AstOptimizer* optimizer, Statement* stmt);
bool ast_optimizer_propagate_assignment_statement(AstOptimizer* optimizer, Statement* stmt);

bool ast_optimizer_propagate_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_constant_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_unary_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_binary_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_group_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_identifier_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_cast_expression(AstOptimizer* optimizer, Expression** expr);
bool ast_optimizer_propagate_incdec_expression(AstOptimizer* optimizer, Expression** expr);

void ast_optimizer_dce(AstOptimizer* optimizer, TranslationUnit* tu);
void ast_optimizer_dce_mark_expression(Expression* expr, Declaration** used);
void ast_optimizer_dce_mark_statement(Statement* stmt, Declaration** used);
bool ast_optimizer_dce_is_used(Declaration* decl, Declaration** used);
