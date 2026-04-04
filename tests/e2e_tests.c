#include "e2e_tests.h"

#include "test_framework.h"

#define EXPECT_RETURN_I64(source, expected_val)                  \
	do                                                           \
	{                                                            \
		PipelineResult _r = run_pipeline(alloc, source, false);  \
		ASSERT_EQ(_r.vm_result, VM_OK);                          \
		ASSERT_EQ(_r.return_value.i64_val, (i64)(expected_val)); \
	} while (0)
#define EXPECT_RETURN_I64_OPT(source, expected_val)              \
	do                                                           \
	{                                                            \
		PipelineResult _r = run_pipeline(alloc, source, true);   \
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

// ---------------------------------------------------------------------------
// If / Else
// ---------------------------------------------------------------------------

TEST(e2e_if_true_branch)
{
	EXPECT_RETURN_I64("var int a = 10;\n"
	                  "var int b = 10;\n"
	                  "if a == b {\n"
	                  "    a += b;\n"
	                  "} else {\n"
	                  "    a -= b;\n"
	                  "}\n"
	                  "return a;",
	                  20);
}

TEST(e2e_if_false_branch)
{
	EXPECT_RETURN_I64("var int a = 10;\n"
	                  "var int b = 20;\n"
	                  "if a == b {\n"
	                  "    a += b;\n"
	                  "} else {\n"
	                  "    a -= b;\n"
	                  "}\n"
	                  "return a;",
	                  -10);
}

TEST(e2e_if_no_else)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "if x == 5 {\n"
	                  "    x += 10;\n"
	                  "}\n"
	                  "return x;",
	                  15);
}

TEST(e2e_if_no_else_false)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "if x == 99 {\n"
	                  "    x += 10;\n"
	                  "}\n"
	                  "return x;",
	                  5);
}

TEST(e2e_if_less_than)
{
	EXPECT_RETURN_I64("var int x = 3;\n"
	                  "var int y = 10;\n"
	                  "if x < y {\n"
	                  "    return x;\n"
	                  "} else {\n"
	                  "    return y;\n"
	                  "}\n",
	                  3);
}

TEST(e2e_if_greater_than)
{
	EXPECT_RETURN_I64("var int x = 3;\n"
	                  "var int y = 10;\n"
	                  "if x > y {\n"
	                  "    return x;\n"
	                  "} else {\n"
	                  "    return y;\n"
	                  "}\n",
	                  10);
}

TEST(e2e_if_multiple_stmts_in_block)
{
	EXPECT_RETURN_I64("var int a = 1;\n"
	                  "var int b = 2;\n"
	                  "if a != b {\n"
	                  "    a += 10;\n"
	                  "    b += 20;\n"
	                  "    a += b;\n"
	                  "}\n"
	                  "return a;",
	                  33);
}

TEST(e2e_if_after_if)
{
	EXPECT_RETURN_I64("var int x = 0;\n"
	                  "if 1 == 1 {\n"
	                  "    x += 10;\n"
	                  "}\n"
	                  "if 2 == 2 {\n"
	                  "    x += 20;\n"
	                  "}\n"
	                  "return x;",
	                  30);
}

TEST(e2e_if_nested)
{
	EXPECT_RETURN_I64("var int x = 5;\n"
	                  "if x > 0 {\n"
	                  "    if x > 3 {\n"
	                  "        x += 100;\n"
	                  "    } else {\n"
	                  "        x += 50;\n"
	                  "    }\n"
	                  "} else {\n"
	                  "    x = 0;\n"
	                  "}\n"
	                  "return x;",
	                  105);
}

TEST(e2e_if_compound_assign_both_branches)
{
	EXPECT_RETURN_I64("var int r = 100;\n"
	                  "var int a = 10;\n"
	                  "var int b = 10;\n"
	                  "if a == b {\n"
	                  "    r *= 2;\n"
	                  "} else {\n"
	                  "    r /= 2;\n"
	                  "}\n"
	                  "return r;",
	                  200);
}

// ---------------------------------------------------------------------------
// While loops
// ---------------------------------------------------------------------------

TEST(e2e_while_basic_sum)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "var int i = 0;\n"
	                  "while i < 10 {\n"
	                  "    sum += i;\n"
	                  "    i += 1;\n"
	                  "}\n"
	                  "return sum;",
	                  45);
}

TEST(e2e_while_zero_iterations)
{
	EXPECT_RETURN_I64("var int x = 42;\n"
	                  "while x < 0 {\n"
	                  "    x = 0;\n"
	                  "}\n"
	                  "return x;",
	                  42);
}

TEST(e2e_while_single_iteration)
{
	EXPECT_RETURN_I64("var int x = 0;\n"
	                  "while x == 0 {\n"
	                  "    x = 99;\n"
	                  "}\n"
	                  "return x;",
	                  99);
}

TEST(e2e_while_countdown)
{
	EXPECT_RETURN_I64("var int n = 5;\n"
	                  "var int result = 1;\n"
	                  "while n > 0 {\n"
	                  "    result *= n;\n"
	                  "    n -= 1;\n"
	                  "}\n"
	                  "return result;",
	                  120);
}

TEST(e2e_while_multiply)
{
	EXPECT_RETURN_I64("var int x = 1;\n"
	                  "var int i = 0;\n"
	                  "while i < 8 {\n"
	                  "    x *= 2;\n"
	                  "    i += 1;\n"
	                  "}\n"
	                  "return x;",
	                  256);
}

TEST(e2e_while_nested)
{
	EXPECT_RETURN_I64("var int total = 0;\n"
	                  "var int i = 0;\n"
	                  "while i < 3 {\n"
	                  "    var int j = 0;\n"
	                  "    while j < 3 {\n"
	                  "        total += 1;\n"
	                  "        j += 1;\n"
	                  "    }\n"
	                  "    i += 1;\n"
	                  "}\n"
	                  "return total;",
	                  9);
}

TEST(e2e_while_with_inner_var)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "var int i = 0;\n"
	                  "while i < 5 {\n"
	                  "    var int tmp = i * 2;\n"
	                  "    sum += tmp;\n"
	                  "    i += 1;\n"
	                  "}\n"
	                  "return sum;",
	                  20);
}

TEST(e2e_while_with_if)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "var int i = 0;\n"
	                  "while i < 10 {\n"
	                  "    if i == 5 {\n"
	                  "        sum += 100;\n"
	                  "    } else {\n"
	                  "        sum += 1;\n"
	                  "    }\n"
	                  "    i += 1;\n"
	                  "}\n"
	                  "return sum;",
	                  109);
}

TEST(e2e_while_compound_condition)
{
	EXPECT_RETURN_I64("var int x = 100;\n"
	                  "while x > 1 {\n"
	                  "    x /= 2;\n"
	                  "}\n"
	                  "return x;",
	                  1);
}

// ---------------------------------------------------------------------------
// For loops
// ---------------------------------------------------------------------------

TEST(e2e_for_basic_sum)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "for var int i = 0; i < 10; i++ {\n"
	                  "    sum += i;\n"
	                  "}\n"
	                  "return sum;",
	                  45);
}

TEST(e2e_for_zero_iterations)
{
	EXPECT_RETURN_I64("var int x = 42;\n"
	                  "for var int i = 0; i > 10; i++ {\n"
	                  "    x = 0;\n"
	                  "}\n"
	                  "return x;",
	                  42);
}

TEST(e2e_for_countdown)
{
	EXPECT_RETURN_I64("var int result = 1;\n"
	                  "for var int n = 5; n > 0; n-- {\n"
	                  "    result *= n;\n"
	                  "}\n"
	                  "return result;",
	                  120);
}

TEST(e2e_for_nested)
{
	EXPECT_RETURN_I64("var int total = 0;\n"
	                  "for var int i = 0; i < 3; i++ {\n"
	                  "    for var int j = 0; j < 4; j++ {\n"
	                  "        total += 1;\n"
	                  "    }\n"
	                  "}\n"
	                  "return total;",
	                  12);
}

TEST(e2e_for_with_if)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "for var int i = 0; i < 10; i++ {\n"
	                  "    if i == 5 {\n"
	                  "        sum += 100;\n"
	                  "    } else {\n"
	                  "        sum += 1;\n"
	                  "    }\n"
	                  "}\n"
	                  "return sum;",
	                  109);
}

TEST(e2e_for_multiply)
{
	EXPECT_RETURN_I64("var int x = 1;\n"
	                  "for var int i = 0; i < 8; i++ {\n"
	                  "    x *= 2;\n"
	                  "}\n"
	                  "return x;",
	                  256);
}

TEST(e2e_for_with_inner_var)
{
	EXPECT_RETURN_I64("var int sum = 0;\n"
	                  "for var int i = 0; i < 5; i++ {\n"
	                  "    var int tmp = i * 2;\n"
	                  "    sum += tmp;\n"
	                  "}\n"
	                  "return sum;",
	                  20);
}

// ---------------------------------------------------------------------------
// Inline for loops
// ---------------------------------------------------------------------------

TEST(e2e_inline_for_basic_sum)
{
	EXPECT_RETURN_I64_OPT("var int sum = 0;\n"
	                      "inline for var int i = 0; i < 5; i++ {\n"
	                      "    sum += i;\n"
	                      "}\n"
	                      "return sum;",
	                      10);
}

TEST(e2e_inline_for_multiply)
{
	EXPECT_RETURN_I64_OPT("var int result = 0;\n"
	                      "inline for var int i = 1; i <= 4; i++ {\n"
	                      "    result += i * 10;\n"
	                      "}\n"
	                      "return result;",
	                      100);
}

TEST(e2e_inline_for_single_iteration)
{
	EXPECT_RETURN_I64_OPT("var int x = 0;\n"
	                      "inline for var int i = 0; i < 1; i++ {\n"
	                      "    x += 42;\n"
	                      "}\n"
	                      "return x;",
	                      42);
}

TEST(e2e_inline_for_countdown)
{
	EXPECT_RETURN_I64_OPT("var int sum = 0;\n"
	                      "inline for var int i = 3; i > 0; i-- {\n"
	                      "    sum += i;\n"
	                      "}\n"
	                      "return sum;",
	                      6);
}

TEST(e2e_inline_for_nested_in_regular_for)
{
	EXPECT_RETURN_I64_OPT("var int result = 0;\n"
	                       "for var int i = 1; i <= 3; i++ {\n"
	                       "    inline for var int j = 1; j <= 3; j++ {\n"
	                       "        result += j;\n"
	                       "    }\n"
	                       "}\n"
	                       "return result;",
	                       18);
}

TEST(e2e_inline_for_with_expression)
{
	EXPECT_RETURN_I64_OPT("var int sum = 0;\n"
	                      "inline for var int i = 0; i < 4; i++ {\n"
	                      "    var int val = i * i;\n"
	                      "    sum += val;\n"
	                      "}\n"
	                      "return sum;",
	                      14);
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

	print_section("If / Else");
	RUN_TEST(e2e_if_true_branch);
	RUN_TEST(e2e_if_false_branch);
	RUN_TEST(e2e_if_no_else);
	RUN_TEST(e2e_if_no_else_false);
	RUN_TEST(e2e_if_less_than);
	RUN_TEST(e2e_if_greater_than);
	RUN_TEST(e2e_if_multiple_stmts_in_block);
	RUN_TEST(e2e_if_after_if);
	RUN_TEST(e2e_if_nested);
	RUN_TEST(e2e_if_compound_assign_both_branches);

	print_section("For loops");
	RUN_TEST(e2e_for_basic_sum);
	RUN_TEST(e2e_for_zero_iterations);
	RUN_TEST(e2e_for_countdown);
	RUN_TEST(e2e_for_nested);
	RUN_TEST(e2e_for_with_if);
	RUN_TEST(e2e_for_multiply);
	RUN_TEST(e2e_for_with_inner_var);

	print_section("Inline for loops");
	RUN_TEST(e2e_inline_for_basic_sum);
	RUN_TEST(e2e_inline_for_multiply);
	RUN_TEST(e2e_inline_for_single_iteration);
	RUN_TEST(e2e_inline_for_countdown);
	RUN_TEST(e2e_inline_for_nested_in_regular_for);
	RUN_TEST(e2e_inline_for_with_expression);

	print_section("While loops");
	RUN_TEST(e2e_while_basic_sum);
	RUN_TEST(e2e_while_zero_iterations);
	RUN_TEST(e2e_while_single_iteration);
	RUN_TEST(e2e_while_countdown);
	RUN_TEST(e2e_while_multiply);
	RUN_TEST(e2e_while_nested);
	RUN_TEST(e2e_while_with_inner_var);
	RUN_TEST(e2e_while_with_if);
	RUN_TEST(e2e_while_compound_condition);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("e2e", total_ms);
}
