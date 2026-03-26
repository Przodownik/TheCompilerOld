#include "sema_tests.h"

#include "test_framework.h"

typedef struct SemaTestResult
{
	bool sema_ok;
	u32 error_count;
	u32 warning_count;
	TranslationUnit tu;
} SemaTestResult;

static SemaTestResult run_sema(Allocator* alloc, const char* source)
{
	SemaTestResult r   = {0};
	File file          = make_test_file(alloc, source);
	Lexer lexer        = lexer_create(&file);
	Parser parser      = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu = parser_parse(&parser);

	diagnostics_reset();
	diagnostics_enable_capture();

	Sema sema       = sema_create(alloc, alloc, &file);
	r.sema_ok       = sema_analyze(&sema, &tu);
	r.tu            = tu;
	r.error_count   = diagnostics_get_error_count();
	r.warning_count = diagnostics_get_warning_count();

	diagnostics_disable_capture();

	return r;
}

// ---------------------------------------------------------------------------
// Error tests
// ---------------------------------------------------------------------------

TEST(sema_error_duplicate_variable)
{
	SemaTestResult r = run_sema(alloc, "var int x = 1;\n"
	                                   "var int x = 2;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Variable 'x' already declared in this scope");
}

TEST(sema_error_undefined_identifier)
{
	SemaTestResult r = run_sema(alloc, "var int x = y;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Undefined identifier 'y'");
}

TEST(sema_error_cyclic_dependency)
{
	SemaTestResult r = run_sema(alloc, "var int x = x;\n");
	ASSERT_FALSE(r.sema_ok);
	ASSERT_TRUE(r.error_count > 0);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Cyclic dependency detected while resolving identifier 'x'");
}

TEST(sema_error_negate_unsigned)
{
	SemaTestResult r = run_sema(alloc, "var uint x = 5;\n"
	                                   "var uint y = -x;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Cannot negate unsigned type 'uint'");
}

TEST(sema_error_ordering_on_bool)
{
	SemaTestResult r = run_sema(alloc, "var bool a = true;\n"
	                                   "var bool b = false;\n"
	                                   "var bool c = a < b;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Cannot use ordering operator '<' on 'bool' operands");
}

TEST(sema_error_incompatible_binary_types)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var bool y = true;\n"
	                                   "var int z = x + y;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Cannot implicitly cast types 'int' and 'bool'");
}

TEST(sema_error_implicit_cast_initializer)
{
	SemaTestResult r = run_sema(alloc, "var int x = 3.14d;\n");
	ASSERT_FALSE(r.sema_ok);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_ERROR);
	ASSERT_STR_CONTAINS(e->message, "Cannot implicitly cast constant of type 'double' to expected type 'int'");
}

TEST(sema_warning_unused_variable)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_TRUE(r.warning_count > 0);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_WARN);
	ASSERT_STR_CONTAINS(e->message, "Variable 'x' is declared but never used");
}

TEST(sema_warning_redundant_cast)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5 as int;\n"
	                                   "return x;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_TRUE(r.warning_count > 0);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_WARN);
	ASSERT_STR_CONTAINS(e->message, "Redundant cast from 'int' to 'int'");
}
TEST(sema_warning_unnecessary_cast)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var long y = x as long;\n"
	                                   "return y;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_TRUE(r.warning_count > 0);

	ASSERT_EQ(diagnostics_captured_count(), 1);
	DiagnosticEntry* e = diagnostics_get_captured(0);
	ASSERT_EQ(e->type, DIAGNOSTIC_PRINT_TYPE_WARN);
	ASSERT_STR_CONTAINS(e->message, "Unnecessary cast: 'int' is implicitly convertible to 'long'");
}

TEST(sema_valid_int_arithmetic)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var int y = 10;\n"
	                                   "return x + y;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_valid_implicit_widening_int_to_long)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var long y = x;\n"
	                                   "return y;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_valid_implicit_widening_float_to_double)
{
	SemaTestResult r = run_sema(alloc, "var float f = 1.0f;\n"
	                                   "var double d = f;\n"
	                                   "return d;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_valid_explicit_cast_int_to_float)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var float f = x as float;\n"
	                                   "return f;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_valid_comparison_returns_bool)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var int y = 10;\n"
	                                   "var bool result = x < y;\n"
	                                   "return result;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_valid_bool_equality)
{
	SemaTestResult r = run_sema(alloc, "var bool a = true;\n"
	                                   "var bool b = false;\n"
	                                   "var bool c = a == b;\n"
	                                   "return c;\n");
	ASSERT_TRUE(r.sema_ok);
	ASSERT_EQ(r.error_count, 0);
}

TEST(sema_inserts_implicit_cast_in_binary)
{
	SemaTestResult r = run_sema(alloc, "var int x = 5;\n"
	                                   "var long y = 10;\n"
	                                   "return x + y;\n");
	ASSERT_TRUE(r.sema_ok);

	Statement* ret_stmt  = r.tu.statements[2];
	Expression* add_expr = ret_stmt->return_stmt.expression;
	ASSERT_EQ(add_expr->type, EXPRESSION_TYPE_BINARY);

	Expression* left = add_expr->binary.left;
	ASSERT_EQ(left->type, EXPRESSION_TYPE_CAST);
	ASSERT_EQ(left->cast.target_type->kind, TYPE_KIND_LONG);
	ASSERT_EQ(left->resolved_type->kind, TYPE_KIND_LONG);
}

TEST(sema_inserts_implicit_cast_in_initializer)
{
	SemaTestResult r = run_sema(alloc, "var long y = 42;\n"
	                                   "return y;\n");
	ASSERT_TRUE(r.sema_ok);

	Statement* decl_stmt = r.tu.statements[0];
	Expression* init     = decl_stmt->decl_stmt.declaration->variable.initializer;
	ASSERT_EQ(init->resolved_type->kind, TYPE_KIND_LONG);
}

int run_sema_tests(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running sema tests..." ANSI_COLOR_RESET "\n");

	print_section("Error tests");
	RUN_TEST(sema_error_duplicate_variable);
	RUN_TEST(sema_error_undefined_identifier);
	RUN_TEST(sema_error_cyclic_dependency);
	RUN_TEST(sema_error_negate_unsigned);
	RUN_TEST(sema_error_ordering_on_bool);
	RUN_TEST(sema_error_incompatible_binary_types);
	RUN_TEST(sema_error_implicit_cast_initializer);

	print_section("Warning tests");
	RUN_TEST(sema_warning_unused_variable);
	RUN_TEST(sema_warning_redundant_cast);
	RUN_TEST(sema_warning_unnecessary_cast);

	print_section("Valid code tests");
	RUN_TEST(sema_valid_int_arithmetic);
	RUN_TEST(sema_valid_implicit_widening_int_to_long);
	RUN_TEST(sema_valid_implicit_widening_float_to_double);
	RUN_TEST(sema_valid_explicit_cast_int_to_float);
	RUN_TEST(sema_valid_comparison_returns_bool);
	RUN_TEST(sema_valid_bool_equality);
	RUN_TEST(sema_inserts_implicit_cast_in_binary);
	RUN_TEST(sema_inserts_implicit_cast_in_initializer);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("sema", total_ms);
}
