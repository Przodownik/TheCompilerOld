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

typedef enum AstOptimizationPass
{
	AST_OPTIMIZATION_PASS_CONSTANT_FOLDING,
	AST_OPTIMIZATION_PASS_COUNT
} AstOptimizationPass;

AstOptimizer ast_optimizer_create(Allocator* expr_alloc);
void ast_optimizer_run(AstOptimizer* optimizer, TranslationUnit* tu);

void ast_optimizer_run_pass(AstOptimizer* optimizer, AstOptimizationPass pass, TranslationUnit* tu);

Expression* ast_optimizer_optimize_expression(AstOptimizer* optimizer, AstOptimizationPass pass, Expression* expr);
Expression* ast_optimizer_constant_fold_expression_pass(AstOptimizer* optimizer, Expression* expr);
