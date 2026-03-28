#include "ast_optimizer_tests.h"

#include "test_framework.h"

static TranslationUnit parse_sema_optimize(Allocator* alloc, const char* src)
{
	File file          = make_test_file(alloc, src);
	Lexer lexer        = lexer_create(&file);
	Parser parser      = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu = parser_parse(&parser);
	Sema sema          = sema_create(alloc, alloc, &file);

	sema_analyze(&sema, &tu);
	AstOptimizer opt = ast_optimizer_create(alloc);
	ast_optimizer_run(&opt, &tu);

	return tu;
}

// ---------------------------------------------------------------------------
// Constant folding
// ---------------------------------------------------------------------------

TEST(fold_integer_add)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 2 + 3;\n"
	                                                "return x;\n");
	Statement* stmt    = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)init->constant.integer_value, 5);
}

TEST(fold_integer_sub)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 10 - 3;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 7);
}

TEST(fold_integer_mul)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 4 * 5;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 20);
}

TEST(fold_integer_div)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 10 / 2;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 5);
}

TEST(fold_nested_arithmetic)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = (2 + 3) * (4 - 1);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 15);
}

TEST(fold_unary_negation)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = -5;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, -5);
}

TEST(fold_double_negation)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = -(-5);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 5);
}

TEST(fold_comparison_gt)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 3 > 2;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_TRUE(init->constant.boolean_value);
}

TEST(fold_comparison_eq_false)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 1 == 2;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_FALSE(init->constant.boolean_value);
}

TEST(fold_float_add)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var float x = 1.5f + 2.5f;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_FLOAT);
	ASSERT_FLOAT_EQ(init->constant.float_value, 4.0f, 0.001f);
}

TEST(fold_double_mul)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var double x = 2.0d * 3.0d;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_DOUBLE);
	ASSERT_FLOAT_EQ(init->constant.double_value, 6.0, 0.001);
}

// ---------------------------------------------------------------------------
// Variable propagation
// ---------------------------------------------------------------------------

TEST(propagate_simple_variable)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 5;\n"
	                                                "var int y = x + 1;\n"
	                                                "return y;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 6);
}

TEST(propagate_chain)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int a = 2;\n"
	                                                "var int b = 3;\n"
	                                                "var int c = a + b;\n"
	                                                "return c;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 5);
}

TEST(dce_removes_unused_after_propagation)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 5;\n"
	                                                "var int y = x;\n"
	                                                "return y;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;

	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)init->constant.integer_value, 5);
}

// ---------------------------------------------------------------------------
// Differential testing (compare optimized vs non-optimized execution results)
// ---------------------------------------------------------------------------

TEST(differential_simple_arithmetic)
{
	const char* src = "return 2 + 3 * 4;";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(differential_variable_propagation)
{
	const char* src = "var int x = 5;\n"
	                  "var int y = x + 10;\n"
	                  "return y;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}
TEST(differential_float_arithmetic)
{
	const char* src = "return 1.5f + 2.5f;";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_FLOAT_EQ(no_opt.return_value.f32_val, with_opt.return_value.f32_val, 0.001f);
}

int run_ast_optimizer_tests(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running ast optimizer tests..." ANSI_COLOR_RESET "\n");

	print_section("Constant folding");
	RUN_TEST(fold_integer_add);
	RUN_TEST(fold_integer_sub);
	RUN_TEST(fold_integer_mul);
	RUN_TEST(fold_integer_div);
	RUN_TEST(fold_nested_arithmetic);
	RUN_TEST(fold_unary_negation);
	RUN_TEST(fold_double_negation);
	RUN_TEST(fold_comparison_gt);
	RUN_TEST(fold_comparison_eq_false);
	RUN_TEST(fold_float_add);
	RUN_TEST(fold_double_mul);

	print_section("Variable propagation");
	RUN_TEST(propagate_simple_variable);
	RUN_TEST(propagate_chain);
	RUN_TEST(dce_removes_unused_after_propagation);

	print_section("Differential testing");
	RUN_TEST(differential_simple_arithmetic);
	RUN_TEST(differential_variable_propagation);
	RUN_TEST(differential_float_arithmetic);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("ast optimizer", total_ms);
}
