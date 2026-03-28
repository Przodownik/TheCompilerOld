#include "e2e_tests.h"

#include "test_framework.h"

#define EXPECT_RETURN_I64(source, expected_val)                  \
	do                                                           \
	{                                                            \
		PipelineResult _r = run_pipeline(alloc, source, false);  \
		ASSERT_EQ(_r.vm_result, VM_OK);                          \
		ASSERT_EQ(_r.return_value.i64_val, (i64)(expected_val)); \
	} while (0)
#define EXPECT_RETURN_BOOL(source, expected_val)                 \
	do                                                           \
	{                                                            \
		PipelineResult _r = run_pipeline(alloc, source, false);  \
		ASSERT_EQ(_r.vm_result, VM_OK);                          \
		ASSERT_EQ(_r.return_value.i64_val, (i64)(expected_val)); \
	} while (0)
#define EXPECT_RETURN_F32(source, expected_val, eps)                 \
	do                                                               \
	{                                                                \
		PipelineResult _r = run_pipeline(alloc, source, false);      \
		ASSERT_EQ(_r.vm_result, VM_OK);                              \
		ASSERT_FLOAT_EQ(_r.return_value.f32_val, expected_val, eps); \
	} while (0)
#define EXPECT_RETURN_F64(source, expected_val, eps)                 \
	do                                                               \
	{                                                                \
		PipelineResult _r = run_pipeline(alloc, source, false);      \
		ASSERT_EQ(_r.vm_result, VM_OK);                              \
		ASSERT_FLOAT_EQ(_r.return_value.f64_val, expected_val, eps); \
	} while (0)

// ---------------------------------------------------------------------------
// Integer arithmetic
// ---------------------------------------------------------------------------

TEST(e2e_add)
{
	EXPECT_RETURN_I64("return 2 + 3;", 5);
}

TEST(e2e_sub)
{
	EXPECT_RETURN_I64("return 10 - 3;", 7);
}

TEST(e2e_mul)
{
	EXPECT_RETURN_I64("return 4 * 5;", 20);
}

TEST(e2e_div)
{
	EXPECT_RETURN_I64("return 10 / 2;", 5);
}

TEST(e2e_precedence)
{
	EXPECT_RETURN_I64("return 2 + 3 * 4;", 14);
}

TEST(e2e_parens)
{
	EXPECT_RETURN_I64("return (2 + 3) * 4;", 20);
}

TEST(e2e_complex_arith)
{
	EXPECT_RETURN_I64("return (10 - 2) * 3 + 1;", 25);
}

// ---------------------------------------------------------------------------
// Variables
// ---------------------------------------------------------------------------

TEST(e2e_var_return)
{
	EXPECT_RETURN_I64("var int x = 42;\nreturn x;", 42);
}

TEST(e2e_var_arithmetic)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "var int y = x + 1;\n"
	                  "return y;\n",
	                  6);
}

TEST(e2e_multi_var)
{
	EXPECT_RETURN_I64("var int a = 10;\n"
	                  "var int b = 20;\n"
	                  "var int c = a + b;\n"
	                  "return c * 2;\n",
	                  60);
}

// ---------------------------------------------------------------------------
// Negation
// ---------------------------------------------------------------------------

TEST(e2e_negate)
{
	EXPECT_RETURN_I64("return -5;", -5);
}

TEST(e2e_double_negate)
{
	EXPECT_RETURN_I64("return -(-5);", 5);
}

TEST(e2e_negate_expr)
{
	EXPECT_RETURN_I64("return -(2 + 3);", -5);
}

// ---------------------------------------------------------------------------
// Comparison operators
// ---------------------------------------------------------------------------

TEST(e2e_eq_true)
{
	EXPECT_RETURN_BOOL("return 5 == 5;", 1);
}

TEST(e2e_eq_false)
{
	EXPECT_RETURN_BOOL("return 5 == 3;", 0);
}

TEST(e2e_neq_true)
{
	EXPECT_RETURN_BOOL("return 5 != 3;", 1);
}

TEST(e2e_neq_false)
{
	EXPECT_RETURN_BOOL("return 5 != 5;", 0);
}

TEST(e2e_lt_true)
{
	EXPECT_RETURN_BOOL("return 3 < 5;", 1);
}

TEST(e2e_lt_false)
{
	EXPECT_RETURN_BOOL("return 5 < 3;", 0);
}

TEST(e2e_gt_true)
{
	EXPECT_RETURN_BOOL("return 5 > 3;", 1);
}

TEST(e2e_gt_false)
{
	EXPECT_RETURN_BOOL("return 3 > 5;", 0);
}

TEST(e2e_leq_eq)
{
	EXPECT_RETURN_BOOL("return 3 <= 3;", 1);
}

TEST(e2e_leq_less)
{
	EXPECT_RETURN_BOOL("return 2 <= 3;", 1);
}

TEST(e2e_geq_eq)
{
	EXPECT_RETURN_BOOL("return 5 >= 5;", 1);
}

TEST(e2e_geq_greater)
{
	EXPECT_RETURN_BOOL("return 5 >= 3;", 1);
}

// ---------------------------------------------------------------------------
// Float / Double
// ---------------------------------------------------------------------------

TEST(e2e_float_add)
{
	EXPECT_RETURN_F32("return 1.5f + 2.5f;", 4.0f, 0.001f);
}

TEST(e2e_float_var)
{
	EXPECT_RETURN_F32("var float x = 1.5f;\n"
	                  "var float y = 2.5f;\n"
	                  "return x + y;\n",
	                  4.0f, 0.001f);
}

TEST(e2e_double_mul)
{
	EXPECT_RETURN_F64("return 2.0d * 3.0d;", 6.0, 0.001);
}

TEST(e2e_double_var)
{
	EXPECT_RETURN_F64("var double d = 2.718d;\n"
	                  "return d;\n",
	                  2.718, 0.001);
}

// ---------------------------------------------------------------------------
// Type casts
// ---------------------------------------------------------------------------

TEST(e2e_cast_int_to_float)
{
	EXPECT_RETURN_F32("var int x = 5;\n"
	                  "return x as float;\n",
	                  5.0f, 0.001f);
}

TEST(e2e_cast_float_to_int)
{
	EXPECT_RETURN_I64("var float f = 3.14f;\n"
	                  "return f as int;\n",
	                  3);
}

// ---------------------------------------------------------------------------
// Boolean variables and comparisons with vars
// ---------------------------------------------------------------------------

TEST(e2e_bool_var)
{
	EXPECT_RETURN_BOOL("var bool b = true;\n"
	                   "return b;\n",
	                   1);
}

TEST(e2e_comparison_with_vars)
{
	EXPECT_RETURN_BOOL("var int x = 5;\n"
	                   "return x == 5;\n",
	                   1);
}

TestResults run_e2e_tests(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running e2e tests..." ANSI_COLOR_RESET "\n");

	print_section("Integer arithmetic");
	RUN_TEST(e2e_add);
	RUN_TEST(e2e_sub);
	RUN_TEST(e2e_mul);
	RUN_TEST(e2e_div);
	RUN_TEST(e2e_precedence);
	RUN_TEST(e2e_parens);
	RUN_TEST(e2e_complex_arith);

	print_section("Variables");
	RUN_TEST(e2e_var_return);
	RUN_TEST(e2e_var_arithmetic);
	RUN_TEST(e2e_multi_var);

	print_section("Negation");
	RUN_TEST(e2e_negate);
	RUN_TEST(e2e_double_negate);
	RUN_TEST(e2e_negate_expr);

	print_section("Comparison operators");
	RUN_TEST(e2e_eq_true);
	RUN_TEST(e2e_eq_false);
	RUN_TEST(e2e_neq_true);
	RUN_TEST(e2e_neq_false);
	RUN_TEST(e2e_lt_true);
	RUN_TEST(e2e_lt_false);
	RUN_TEST(e2e_gt_true);
	RUN_TEST(e2e_gt_false);
	RUN_TEST(e2e_leq_eq);
	RUN_TEST(e2e_leq_less);
	RUN_TEST(e2e_geq_eq);
	RUN_TEST(e2e_geq_greater);

	print_section("Float / Double");
	RUN_TEST(e2e_float_add);
	RUN_TEST(e2e_float_var);
	RUN_TEST(e2e_double_mul);
	RUN_TEST(e2e_double_var);

	print_section("Type casts");
	RUN_TEST(e2e_cast_int_to_float);
	RUN_TEST(e2e_cast_float_to_int);

	print_section("Boolean variables and comparisons with vars");
	RUN_TEST(e2e_bool_var);
	RUN_TEST(e2e_comparison_with_vars);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("e2e", total_ms);
}
