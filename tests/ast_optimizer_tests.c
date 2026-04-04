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
	AstOptimizer opt = ast_optimizer_create(alloc, alloc, alloc);
	ast_optimizer_run(&opt, &tu, true);

	return tu;
}

static TranslationUnit parse_sema_unroll_only(Allocator* alloc, const char* src)
{
	File file          = make_test_file(alloc, src);
	Lexer lexer        = lexer_create(&file);
	Parser parser      = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu = parser_parse(&parser);
	Sema sema          = sema_create(alloc, alloc, &file);

	sema_analyze(&sema, &tu);
	AstOptimizer opt = ast_optimizer_create(alloc, alloc, alloc);
	ast_optimizer_run(&opt, &tu, false);

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

TEST(fold_unary_negate_grouped_constant)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = -(42);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)ret->constant.integer_value, -42);
}

TEST(fold_unary_negate_nested_group)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = -((7));\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)ret->constant.integer_value, -7);
}

TEST(fold_binary_add_grouped_operands)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = (2) + (3);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)ret->constant.integer_value, 5);
}

TEST(fold_binary_mul_left_grouped)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = (4) * 5;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)ret->constant.integer_value, 20);
}

TEST(fold_binary_sub_right_grouped)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 10 - (3);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ((i64)ret->constant.integer_value, 7);
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

TEST(fold_comparison_lt_true)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 2 < 3;\nreturn x;\n");

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

TEST(fold_comparison_neq_true)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 1 != 2;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_TRUE(init->constant.boolean_value);
}

TEST(fold_comparison_leq_true)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 3 <= 3;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_TRUE(init->constant.boolean_value);
}

TEST(fold_comparison_geq_false)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = 2 >= 3;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
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

TEST(fold_float_sub)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var float x = 5.0f - 2.0f;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_FLOAT_EQ(init->constant.float_value, 3.0f, 0.001f);
}

TEST(fold_float_div)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var float x = 10.0f / 4.0f;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_FLOAT_EQ(init->constant.float_value, 2.5f, 0.001f);
}

TEST(fold_float_mul)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var float x = 2.0f * 3.0f;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_FLOAT_EQ(init->constant.float_value, 6.0f, 0.001f);
}

TEST(fold_float_negation)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var float x = -3.14f;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_FLOAT_EQ(init->constant.float_value, -3.14f, 0.001f);
}

TEST(fold_double_add)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var double x = 1.5d + 2.5d;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_FLOAT_EQ(init->constant.double_value, 4.0, 0.001);
}

TEST(fold_double_sub)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var double x = 5.0d - 2.0d;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_FLOAT_EQ(init->constant.double_value, 3.0, 0.001);
}

TEST(fold_double_div)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var double x = 10.0d / 4.0d;\nreturn x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* init = stmt->return_stmt.expression;
	ASSERT_FLOAT_EQ(init->constant.double_value, 2.5, 0.001);
}

TEST(fold_float_comparison_grouped)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var bool x = (1.5f) < (2.5f);\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_TRUE(ret->constant.boolean_value);
}

TEST(fold_cast_grouped_float_to_int)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = (5.0f) as int;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)ret->constant.integer_value, 5);
}

TEST(fold_signed_comparison_lt_negative)
{
	const char* src = "var int a = -1;\n"
	                  "var int b = 5;\n"
	                  "var bool r = a < b;\n"
	                  "return r;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(fold_signed_comparison_gt_negatives)
{
	const char* src = "var int a = -1;\n"
	                  "var int b = -5;\n"
	                  "var bool r = a > b;\n"
	                  "return r;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(fold_signed_comparison_leq_negative)
{
	const char* src = "var int a = -3;\n"
	                  "var int b = -3;\n"
	                  "var bool r = a <= b;\n"
	                  "return r;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(fold_signed_division_negative)
{
	const char* src = "var int a = -10;\n"
	                  "var int b = 2;\n"
	                  "var int r = a / b;\n"
	                  "return r;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(fold_signed_division_negative_divisor)
{
	const char* src = "var int a = 10;\n"
	                  "var int b = -2;\n"
	                  "var int r = a / b;\n"
	                  "return r;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(fold_negative_lt_positive_ast)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int a = -1;\n"
	                                                "var int b = 5;\n"
	                                                "var bool r = a < b;\n"
	                                                "return r;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_TRUE(ret->constant.boolean_value);
}

TEST(fold_cast_already_constant_returns_true)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 5.0f as int;\n"
	                                                "return x;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)ret->constant.integer_value, 5);
}

TEST(fold_cast_constant_then_propagate)
{
	TranslationUnit tu = parse_sema_optimize(alloc, "var int x = 5.0f as int;\n"
	                                                "var int y = x + 1;\n"
	                                                "return y;\n");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* ret = stmt->return_stmt.expression;
	ASSERT_EQ(ret->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(ret->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ((i64)ret->constant.integer_value, 6);
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

TEST(differential_cast_expression)
{
	const char* src = "var int x = 5;\nreturn x as float;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_FLOAT_EQ(no_opt.return_value.f32_val, with_opt.return_value.f32_val, 0.001f);
}

TEST(differential_comparison)
{
	const char* src = "var int x = 5;\nvar int y = 3;\nreturn x > y;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(differential_double_arithmetic)
{
	const char* src = "return 2.0d * 3.0d + 1.0d;";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_FLOAT_EQ(no_opt.return_value.f64_val, with_opt.return_value.f64_val, 0.001);
}

TEST(differential_unary_negate_grouped)
{
	const char* src = "return -(10);";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(differential_binary_grouped_operands)
{
	const char* src = "return (2) + (3) * (4);";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(differential_cast_grouped_expression)
{
	const char* src = "return (3.14f) as int;";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

TEST(differential_chained_cast_propagation)
{
	const char* src = "var int x = 5.0f as int;\n"
	                  "var int y = x + 1;\n"
	                  "return y;\n";

	PipelineResult no_opt   = run_pipeline(alloc, src, false);
	PipelineResult with_opt = run_pipeline(alloc, src, true);

	ASSERT_EQ(no_opt.vm_result, VM_OK);
	ASSERT_EQ(with_opt.vm_result, VM_OK);
	ASSERT_EQ(no_opt.return_value.i64_val, with_opt.return_value.i64_val);
}

// ---------------------------------------------------------------------------
// Inline for unrolling
// ---------------------------------------------------------------------------

TEST(inline_for_replaces_with_block)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "inline for var int i = 0; i < 3; i++ {\n"
	    "    var int x = i;\n"
	    "}\n");

	// The inline for should be replaced with a single outer block
	ASSERT_EQ(vector_get_length(tu.statements), 1);

	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);

	// 3 iterations: i=0,1,2
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 3);

	// Each iteration should be a block
	for (u64 k = 0; k < 3; k++)
	{
		Statement* inner = outer->block_stmt.statements[k];
		ASSERT_EQ(inner->type, STATEMENT_TYPE_BLOCK);
	}
}

TEST(inline_for_substitutes_loop_var)
{
	TranslationUnit tu = parse_sema_optimize(alloc,
	    "inline for var int i = 0; i < 3; i++ {\n"
	    "    var int x = i * 10;\n"
	    "}\n");

	// After optimization (fold+propagate+DCE), the inline for body
	// should have constants substituted for 'i'. The outer block remains.
	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 3);

	// Each inner block has a declaration with a folded constant
	for (u64 k = 0; k < 3; k++)
	{
		Statement* inner = outer->block_stmt.statements[k];
		ASSERT_EQ(inner->type, STATEMENT_TYPE_BLOCK);

		Statement* decl_stmt = inner->block_stmt.statements[0];
		ASSERT_EQ(decl_stmt->type, STATEMENT_TYPE_DECLARATION);

		Expression* init = decl_stmt->decl_stmt.declaration->variable.initializer;
		ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
		ASSERT_EQ(init->constant.kind, CONSTANT_KIND_INTEGER);
		ASSERT_EQ((i64)init->constant.integer_value, (i64)(k * 10));
	}
}

TEST(inline_for_single_iteration)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "inline for var int i = 0; i < 1; i++ {\n"
	    "    var int x = i;\n"
	    "}\n");

	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 1);
}

TEST(inline_for_countdown)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "inline for var int i = 3; i > 0; i-- {\n"
	    "    var int x = i;\n"
	    "}\n");

	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);

	// 3 iterations: i=3,2,1
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 3);
}

TEST(inline_for_multiple_body_stmts)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "inline for var int i = 0; i < 2; i++ {\n"
	    "    var int a = i;\n"
	    "    var int b = i;\n"
	    "}\n");

	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 2);

	// Each inner block should have 2 statements
	for (u64 k = 0; k < 2; k++)
	{
		Statement* inner = outer->block_stmt.statements[k];
		ASSERT_EQ(inner->type, STATEMENT_TYPE_BLOCK);
		ASSERT_EQ(vector_get_length(inner->block_stmt.statements), 2);
	}
}

TEST(inline_for_preserves_non_inline_stmts)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "var int y = 42;\n"
	    "inline for var int i = 0; i < 2; i++ {\n"
	    "    var int x = i;\n"
	    "}\n"
	    "return y;\n");

	// 3 top-level statements: var decl, unrolled block, return
	ASSERT_EQ(vector_get_length(tu.statements), 3);

	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_DECLARATION);
	ASSERT_EQ(tu.statements[1]->type, STATEMENT_TYPE_BLOCK);
	ASSERT_EQ(tu.statements[2]->type, STATEMENT_TYPE_RETURN);
}

TEST(inline_for_nested_unroll_structure)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "inline for var int i = 0; i < 2; i++ {\n"
	    "    inline for var int j = 0; j < 3; j++ {\n"
	    "        var int x = i;\n"
	    "    }\n"
	    "}\n");

	// Outer: 1 top-level block
	ASSERT_EQ(vector_get_length(tu.statements), 1);

	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);
	// 2 iterations of i
	ASSERT_EQ(vector_get_length(outer->block_stmt.statements), 2);

	for (u64 i = 0; i < 2; i++)
	{
		Statement* i_block = outer->block_stmt.statements[i];
		ASSERT_EQ(i_block->type, STATEMENT_TYPE_BLOCK);

		// The inner inline for should have been unrolled into a block too
		// i_block contains: the unrolled inner inline for (now a block with 3 children)
		ASSERT_EQ(vector_get_length(i_block->block_stmt.statements), 1);

		Statement* j_outer = i_block->block_stmt.statements[0];
		ASSERT_EQ(j_outer->type, STATEMENT_TYPE_BLOCK);
		// 3 iterations of j
		ASSERT_EQ(vector_get_length(j_outer->block_stmt.statements), 3);

		for (u64 j = 0; j < 3; j++)
		{
			Statement* j_block = j_outer->block_stmt.statements[j];
			ASSERT_EQ(j_block->type, STATEMENT_TYPE_BLOCK);
			ASSERT_EQ(vector_get_length(j_block->block_stmt.statements), 1);
		}
	}
}

TEST(inline_for_nested_substitution)
{
	TranslationUnit tu = parse_sema_optimize(alloc,
	    "inline for var int i = 0; i < 2; i++ {\n"
	    "    inline for var int j = 0; j < 2; j++ {\n"
	    "        var int x = i * 10 + j;\n"
	    "    }\n"
	    "}\n");

	// After full optimization, verify that both i and j are substituted as constants.
	// Structure: outer_block -> 2 i_blocks -> each has 1 j_outer -> 2 j_blocks -> decl
	Statement* outer = tu.statements[0];
	ASSERT_EQ(outer->type, STATEMENT_TYPE_BLOCK);

	i64 expected[4] = {0, 1, 10, 11}; // i=0,j=0 -> 0; i=0,j=1 -> 1; i=1,j=0 -> 10; i=1,j=1 -> 11
	u64 idx         = 0;

	for (u64 i = 0; i < 2; i++)
	{
		Statement* i_block = outer->block_stmt.statements[i];
		Statement* j_outer = i_block->block_stmt.statements[0];

		for (u64 j = 0; j < 2; j++)
		{
			Statement* j_block   = j_outer->block_stmt.statements[j];
			Statement* decl_stmt = j_block->block_stmt.statements[0];
			ASSERT_EQ(decl_stmt->type, STATEMENT_TYPE_DECLARATION);

			Expression* init = decl_stmt->decl_stmt.declaration->variable.initializer;
			ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
			ASSERT_EQ(init->constant.kind, CONSTANT_KIND_INTEGER);
			ASSERT_EQ((i64)init->constant.integer_value, expected[idx]);
			idx++;
		}
	}
}

TEST(inline_for_nested_inside_regular_for_unrolls)
{
	TranslationUnit tu = parse_sema_unroll_only(alloc,
	    "var int sum = 0;\n"
	    "for var int i = 0; i < 3; i++ {\n"
	    "    inline for var int j = 0; j < 2; j++ {\n"
	    "        sum += j;\n"
	    "    }\n"
	    "}\n"
	    "return sum;\n");

	// Top-level: var decl, regular for, return
	ASSERT_EQ(vector_get_length(tu.statements), 3);
	ASSERT_EQ(tu.statements[1]->type, STATEMENT_TYPE_FOR);

	// The for body should contain the unrolled inline for (now a block)
	Statement* for_body = tu.statements[1]->for_stmt.body;
	ASSERT_EQ(for_body->type, STATEMENT_TYPE_BLOCK);
	ASSERT_EQ(vector_get_length(for_body->block_stmt.statements), 1);

	Statement* unrolled = for_body->block_stmt.statements[0];
	ASSERT_EQ(unrolled->type, STATEMENT_TYPE_BLOCK);
	// 2 iterations of j
	ASSERT_EQ(vector_get_length(unrolled->block_stmt.statements), 2);
}

TEST(differential_inline_for_sum)
{
	// Compare: manually unrolled vs inline for
	const char* manual = "var int sum = 0;\n"
	                     "var int x0 = 0 * 10;\n"
	                     "sum += x0;\n"
	                     "var int x1 = 1 * 10;\n"
	                     "sum += x1;\n"
	                     "var int x2 = 2 * 10;\n"
	                     "sum += x2;\n"
	                     "return sum;\n";

	const char* with_inline = "var int sum = 0;\n"
	                          "inline for var int i = 0; i < 3; i++ {\n"
	                          "    var int x = i * 10;\n"
	                          "    sum += x;\n"
	                          "}\n"
	                          "return sum;\n";

	PipelineResult manual_r = run_pipeline(alloc, manual, true);
	PipelineResult inline_r = run_pipeline(alloc, with_inline, true);

	ASSERT_EQ(manual_r.vm_result, VM_OK);
	ASSERT_EQ(inline_r.vm_result, VM_OK);
	ASSERT_EQ(manual_r.return_value.i64_val, inline_r.return_value.i64_val);
}

TestResults run_ast_optimizer_tests(void)
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
	RUN_TEST(fold_unary_negate_grouped_constant);
	RUN_TEST(fold_unary_negate_nested_group);
	RUN_TEST(fold_binary_add_grouped_operands);
	RUN_TEST(fold_binary_mul_left_grouped);
	RUN_TEST(fold_binary_sub_right_grouped);
	RUN_TEST(fold_comparison_gt);
	RUN_TEST(fold_comparison_lt_true);
	RUN_TEST(fold_comparison_eq_false);
	RUN_TEST(fold_comparison_neq_true);
	RUN_TEST(fold_comparison_leq_true);
	RUN_TEST(fold_comparison_geq_false);
	RUN_TEST(fold_float_add);
	RUN_TEST(fold_double_mul);
	RUN_TEST(fold_float_sub);
	RUN_TEST(fold_float_div);
	RUN_TEST(fold_float_mul);
	RUN_TEST(fold_float_negation);
	RUN_TEST(fold_double_add);
	RUN_TEST(fold_double_sub);
	RUN_TEST(fold_double_div);
	RUN_TEST(fold_float_comparison_grouped);
	RUN_TEST(fold_cast_grouped_float_to_int);
	RUN_TEST(fold_signed_comparison_lt_negative);
	RUN_TEST(fold_signed_comparison_gt_negatives);
	RUN_TEST(fold_signed_comparison_leq_negative);
	RUN_TEST(fold_signed_division_negative);
	RUN_TEST(fold_signed_division_negative_divisor);
	RUN_TEST(fold_negative_lt_positive_ast);
	RUN_TEST(fold_cast_already_constant_returns_true);
	RUN_TEST(fold_cast_constant_then_propagate);

	print_section("Variable propagation");
	RUN_TEST(propagate_simple_variable);
	RUN_TEST(propagate_chain);
	RUN_TEST(dce_removes_unused_after_propagation);

	print_section("Inline for unrolling");
	RUN_TEST(inline_for_replaces_with_block);
	RUN_TEST(inline_for_substitutes_loop_var);
	RUN_TEST(inline_for_single_iteration);
	RUN_TEST(inline_for_countdown);
	RUN_TEST(inline_for_multiple_body_stmts);
	RUN_TEST(inline_for_preserves_non_inline_stmts);
	RUN_TEST(inline_for_nested_unroll_structure);
	RUN_TEST(inline_for_nested_substitution);
	RUN_TEST(inline_for_nested_inside_regular_for_unrolls);

	print_section("Differential testing");
	RUN_TEST(differential_simple_arithmetic);
	RUN_TEST(differential_variable_propagation);
	RUN_TEST(differential_float_arithmetic);
	RUN_TEST(differential_cast_expression);
	RUN_TEST(differential_comparison);
	RUN_TEST(differential_double_arithmetic);
	RUN_TEST(differential_unary_negate_grouped);
	RUN_TEST(differential_binary_grouped_operands);
	RUN_TEST(differential_cast_grouped_expression);
	RUN_TEST(differential_chained_cast_propagation);
	RUN_TEST(differential_inline_for_sum);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("ast optimizer", total_ms);
}
