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

TEST(e2e_uint_add)
{
	EXPECT_RETURN_I64("var uint a = 10;\n"
	                  "var uint b = 20;\n"
	                  "return a + b;\n",
	                  30);
}

TEST(e2e_uint_sub)
{
	EXPECT_RETURN_I64("var uint a = 20;\n"
	                  "var uint b = 10;\n"
	                  "return a - b;\n",
	                  10);
}

TEST(e2e_uint_mul)
{
	EXPECT_RETURN_I64("var uint a = 5;\n"
	                  "var uint b = 6;\n"
	                  "return a * b;\n",
	                  30);
}

TEST(e2e_uint_div)
{
	EXPECT_RETURN_I64("var uint a = 20;\n"
	                  "var uint b = 4;\n"
	                  "return a / b;\n",
	                  5);
}

TEST(e2e_char_arithmetic)
{
	EXPECT_RETURN_I64("var char a = 10;\n"
	                  "var char b = 20;\n"
	                  "return a + b;\n",
	                  30);
}

TEST(e2e_short_arithmetic)
{
	EXPECT_RETURN_I64("var short a = 100;\n"
	                  "var short b = 50;\n"
	                  "return a - b;\n",
	                  50);
}

// ---------------------------------------------------------------------------
// Floating-point arithmetic
// ---------------------------------------------------------------------------

TEST(e2e_float_sub)
{
	EXPECT_RETURN_F32("return 5.0f - 2.0f;", 3.0f, 0.001f);
}

TEST(e2e_float_mul)
{
	EXPECT_RETURN_F32("return 2.0f * 3.0f;", 6.0f, 0.001f);
}

TEST(e2e_float_div)
{
	EXPECT_RETURN_F32("return 10.0f / 4.0f;", 2.5f, 0.001f);
}

TEST(e2e_double_add)
{
	EXPECT_RETURN_F64("return 1.5d + 2.5d;", 4.0, 0.001);
}

TEST(e2e_double_sub)
{
	EXPECT_RETURN_F64("return 5.0d - 2.0d;", 3.0, 0.001);
}

TEST(e2e_double_div)
{
	EXPECT_RETURN_F64("return 10.0d / 4.0d;", 2.5, 0.001);
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

TEST(e2e_float_negate)
{
	EXPECT_RETURN_F32("return -3.14f;", -3.14f, 0.001f);
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

TEST(e2e_leq_false)
{
	EXPECT_RETURN_BOOL("return 5 <= 3;", 0);
}

TEST(e2e_geq_false)
{
	EXPECT_RETURN_BOOL("return 3 >= 5;", 0);
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

TEST(e2e_cast_double_to_int)
{
	EXPECT_RETURN_I64("var double d = 3.99d;\n"
	                  "return d as int;\n",
	                  3);
}

TEST(e2e_cast_int_to_double)
{
	EXPECT_RETURN_F64("var int x = 42;\n"
	                  "return x as double;\n",
	                  42.0, 0.001);
}

TEST(e2e_cast_bool_to_int)
{
	EXPECT_RETURN_I64("var bool b = true;\n"
	                  "return b as int;\n",
	                  1);
}

TEST(e2e_cast_float_to_double)
{
	EXPECT_RETURN_F64("var float f = 3.14f;\n"
	                  "var double d = f;\n"
	                  "return d;\n",
	                  3.14, 0.01);
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

TEST(e2e_bool_false_var)
{
	EXPECT_RETURN_BOOL("var bool b = false;\nreturn b;\n", 0);
}

TEST(e2e_comparison_with_vars)
{
	EXPECT_RETURN_BOOL("var int x = 5;\n"
	                   "return x == 5;\n",
	                   1);
}

// ---------------------------------------------------------------------------
// Assignments and increments/decrements
// ---------------------------------------------------------------------------

TEST(simple_assignment)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "x = 10;\n"
	                  "return x;",
	                  10);
}

TEST(compound_add_assign)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "x += 3;\n"
	                  "return x;",
	                  8);
}

TEST(compound_sub_assign)
{

	EXPECT_RETURN_I64("var int x = 10;\n"
	                  "x -= 4;\n"
	                  "return x;",
	                  6);
}

TEST(compound_mul_assign)
{
	EXPECT_RETURN_I64("var int x = 3;\n"
	                  "x *= 4;\n"
	                  "return x;",
	                  12);
}

TEST(compound_div_assign)
{
	EXPECT_RETURN_I64("var int x = 20;\n"
	                  "x /= 5;\n"
	                  "return x;",
	                  4);
}

TEST(e2e_prefix_increment)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "return ++x;",
	                  6);
}

TEST(e2e_postfix_increment)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "return x++;",
	                  5);
}

TEST(e2e_prefix_decrement)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "return --x;",
	                  4);
}

TEST(e2e_postfix_decrement)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "return x--;",
	                  5);
}

TEST(e2e_increment_as_statement)
{
	EXPECT_RETURN_I64("var int x = 0;\n"
	                  "++x;\n"
	                  "++x;\n"
	                  "++x;\n"
	                  "return x;",
	                  3);
}

TEST(e2e_postfix_increment_then_return)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "x++;\n"
	                  "return x;",
	                  6);
}

TEST(e2e_float_compound_assign)
{
	EXPECT_RETURN_F32("var float f = 1.0f;\n"
	                  "f += 0.5f;\n"
	                  "return f;",
	                  1.5f, 0.001f);
}

TEST(e2e_multiple_reassignments)
{
	EXPECT_RETURN_I64("var int x = 1;\n"
	                  "x = 2;\n"
	                  "x = 3;\n"
	                  "x = 4;\n"
	                  "return x;",
	                  4);
}

TEST(e2e_mixed_assignment_and_incdec)
{
	EXPECT_RETURN_I64("var int x = 10;\n"
	                  "x += 5;\n"
	                  "++x;\n"
	                  "x -= 2;\n"
	                  "x--;\n"
	                  "return x;",
	                  13);
}

TEST(e2e_all_compound_operators)
{
	EXPECT_RETURN_I64("var int a = 10;\n"
	                  "a += 5;\n"
	                  "a -= 3;\n"
	                  "a *= 2;\n"
	                  "a /= 6;\n"
	                  "return a;",
	                  4);
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
	RUN_TEST(e2e_uint_add);
	RUN_TEST(e2e_uint_sub);
	RUN_TEST(e2e_uint_mul);
	RUN_TEST(e2e_uint_div);
	RUN_TEST(e2e_char_arithmetic);
	RUN_TEST(e2e_short_arithmetic);

	print_section("Floating-point arithmetic");
	RUN_TEST(e2e_float_sub);
	RUN_TEST(e2e_float_mul);
	RUN_TEST(e2e_float_div);
	RUN_TEST(e2e_double_add);
	RUN_TEST(e2e_double_sub);
	RUN_TEST(e2e_double_div);

	print_section("Variables");
	RUN_TEST(e2e_var_return);
	RUN_TEST(e2e_var_arithmetic);
	RUN_TEST(e2e_multi_var);

	print_section("Negation");
	RUN_TEST(e2e_negate);
	RUN_TEST(e2e_float_negate);
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
	RUN_TEST(e2e_leq_false);
	RUN_TEST(e2e_geq_false);

	print_section("Float / Double");
	RUN_TEST(e2e_float_add);
	RUN_TEST(e2e_float_var);
	RUN_TEST(e2e_double_mul);
	RUN_TEST(e2e_double_var);

	print_section("Type casts");
	RUN_TEST(e2e_cast_int_to_float);
	RUN_TEST(e2e_cast_float_to_int);
	RUN_TEST(e2e_cast_double_to_int);
	RUN_TEST(e2e_cast_int_to_double);
	RUN_TEST(e2e_cast_bool_to_int);
	RUN_TEST(e2e_cast_float_to_double);

	print_section("Boolean variables and comparisons with vars");
	RUN_TEST(e2e_bool_var);
	RUN_TEST(e2e_bool_false_var);
	RUN_TEST(e2e_comparison_with_vars);

	print_section("Assignments and increments/decrements");
	RUN_TEST(simple_assignment);
	RUN_TEST(compound_add_assign);
	RUN_TEST(compound_sub_assign);
	RUN_TEST(compound_mul_assign);
	RUN_TEST(compound_div_assign);
	RUN_TEST(e2e_prefix_increment);
	RUN_TEST(e2e_postfix_increment);
	RUN_TEST(e2e_prefix_decrement);
	RUN_TEST(e2e_postfix_decrement);
	RUN_TEST(e2e_increment_as_statement);
	RUN_TEST(e2e_postfix_increment_then_return);
	RUN_TEST(e2e_float_compound_assign);
	RUN_TEST(e2e_multiple_reassignments);
	RUN_TEST(e2e_mixed_assignment_and_incdec);
	RUN_TEST(e2e_all_compound_operators);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("e2e", total_ms);
}
