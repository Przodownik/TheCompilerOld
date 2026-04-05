/**
 * @file test_framework.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast_opt.h"
#include "wandelt/bytecode.h"
#include "wandelt/defines.h"
#include "wandelt/diagnostics.h"
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/parser.h"
#include "wandelt/platform.h"
#include "wandelt/sema.h"
#include "wandelt/vector.h"
#include "wandelt/vm.h"

typedef struct TestResults
{
	int run;
	int passed;
	int failed;
	double time_ms;
} TestResults;

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

static void print_section(const char* name)
{
	printf("\n  " ANSI_COLOR_CYAN ANSI_COLOR_BOLD "%s" ANSI_COLOR_RESET "\n", name);
}

#define TEST(name)                                                                                                   \
	static void test_##name(Allocator* alloc);                                                                       \
	static void run_test_##name(Allocator* alloc)                                                                    \
	{                                                                                                                \
		g_tests_run++;                                                                                               \
		int _failed_before = g_tests_failed;                                                                         \
		PlatformTimer _timer;                                                                                        \
		platform_timer_start(&_timer);                                                                               \
		printf("  %-50s", #name);                                                                                    \
		test_##name(alloc);                                                                                          \
		double _ms = platform_timer_elapsed_ms(&_timer);                                                             \
		if (g_tests_failed == _failed_before)                                                                        \
		{                                                                                                            \
			g_tests_passed++;                                                                                        \
			printf(ANSI_COLOR_GREEN "PASS" ANSI_COLOR_RESET ANSI_COLOR_DIM "  (%.3fms)" ANSI_COLOR_RESET "\n", _ms); \
		}                                                                                                            \
		else                                                                                                         \
		{                                                                                                            \
			printf(ANSI_COLOR_DIM "  (%.3fms)" ANSI_COLOR_RESET "\n", _ms);                                          \
		}                                                                                                            \
	}                                                                                                                \
	static void test_##name(Allocator* alloc)

#define ASSERT_EQ(a, b)                                                   \
	do                                                                    \
	{                                                                     \
		if ((a) != (b))                                                   \
		{                                                                 \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"            \
			                      "    %s:%d: expected %lld, got %lld\n", \
			       __FILE__, __LINE__, (long long)(b), (long long)(a));   \
			g_tests_failed++;                                             \
			return;                                                       \
		}                                                                 \
	} while (0)

#define ASSERT_TRUE(cond)                                                  \
	do                                                                     \
	{                                                                      \
		if (!(cond))                                                       \
		{                                                                  \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"             \
			                      "    %s:%d: expected true, got false\n", \
			       __FILE__, __LINE__);                                    \
			g_tests_failed++;                                              \
			return;                                                        \
		}                                                                  \
	} while (0)

#define ASSERT_NOT_NULL(ptr)                                                  \
	do                                                                        \
	{                                                                         \
		if ((ptr) == NULL)                                                    \
		{                                                                     \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"                \
			                      "    %s:%d: expected non-null, got NULL\n", \
			       __FILE__, __LINE__);                                       \
			g_tests_failed++;                                                 \
			return;                                                           \
		}                                                                     \
	} while (0)

#define ASSERT_STMT(stmt)                                                          \
	do                                                                             \
	{                                                                              \
		if ((stmt) == NULL || (stmt)->type == STATEMENT_TYPE_INVALID)              \
		{                                                                          \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"                     \
			                      "    %s:%d: expected valid statement, got %s\n", \
			       __FILE__, __LINE__, (stmt) == NULL ? "NULL" : "INVALID");       \
			g_tests_failed++;                                                      \
			return;                                                                \
		}                                                                          \
	} while (0)

#define ASSERT_DECL(decl)                                                            \
	do                                                                               \
	{                                                                                \
		if ((decl) == NULL || (decl)->type == DECLARATION_TYPE_INVALID)              \
		{                                                                            \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"                       \
			                      "    %s:%d: expected valid declaration, got %s\n", \
			       __FILE__, __LINE__, (decl) == NULL ? "NULL" : "INVALID");         \
			g_tests_failed++;                                                        \
			return;                                                                  \
		}                                                                            \
	} while (0)

#define ASSERT_FALSE(cond)                                                 \
	do                                                                     \
	{                                                                      \
		if ((cond))                                                        \
		{                                                                  \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"             \
			                      "    %s:%d: expected false, got true\n", \
			       __FILE__, __LINE__);                                    \
			g_tests_failed++;                                              \
			return;                                                        \
		}                                                                  \
	} while (0)

#define ASSERT_STR_EQ(sv, expected)                                             \
	do                                                                          \
	{                                                                           \
		const char* _exp = (expected);                                          \
		u64 _exp_len     = strlen(_exp);                                        \
		if ((sv).len != _exp_len || memcmp((sv).data, _exp, _exp_len) != 0)     \
		{                                                                       \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"                  \
			                      "    %s:%d: expected \"%s\", got \"%.*s\"\n", \
			       __FILE__, __LINE__, _exp, (int)(sv).len, (sv).data);         \
			g_tests_failed++;                                                   \
			return;                                                             \
		}                                                                       \
	} while (0)

#define ASSERT_STR_CONTAINS(str, expected)                                  \
	do                                                                      \
	{                                                                       \
		const char* _str = (str);                                           \
		const char* _exp = (expected);                                      \
		if (_str == NULL || strstr(_str, _exp) == NULL)                     \
		{                                                                   \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"              \
			                      "    %s:%d: expected to contain \"%s\"\n" \
			                      "    got: \"%s\"\n",                      \
			       __FILE__, __LINE__, _exp, _str ? _str : "(null)");       \
			g_tests_failed++;                                               \
			return;                                                         \
		}                                                                   \
	} while (0)

#define ASSERT_FLOAT_EQ(a, b, epsilon)                                          \
	do                                                                          \
	{                                                                           \
		double _a    = (double)(a);                                             \
		double _b    = (double)(b);                                             \
		double _diff = _a - _b;                                                 \
		if (_diff < 0)                                                          \
			_diff = -_diff;                                                     \
		if (_diff > (double)(epsilon))                                          \
		{                                                                       \
			printf(ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET "\n"                  \
			                      "    %s:%d: expected %f, got %f (diff=%f)\n", \
			       __FILE__, __LINE__, _b, _a, _diff);                          \
			g_tests_failed++;                                                   \
			return;                                                             \
		}                                                                       \
	} while (0)

#define RUN_TEST(name) run_test_##name(&arena)

static File make_test_file(Allocator* alloc, const char* source)
{
	File f;
	f.alloc        = alloc;
	f.path         = string_from_cstr(alloc, "<test>");
	f.name         = string_from_cstr(alloc, "<test>");
	f.content      = string_from_cstr(alloc, source);
	f.content_size = (u64)strlen(source);

	return f;
}

typedef struct PipelineResult
{
	bool parse_ok;
	bool sema_ok;
	u32 error_count;
	u32 warning_count;
	TranslationUnit tu;
	Chunk chunk;
	VmResult vm_result;
	Value return_value;
} PipelineResult;

static PipelineResult run_pipeline(Allocator* alloc, const char* source, bool optimize)
{
	PipelineResult r = {0};

	diagnostics_reset();

	File file          = make_test_file(alloc, source);
	Lexer lexer        = lexer_create(&file);
	Parser parser      = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu = parser_parse(&parser);
	r.tu               = tu;

	if (diagnostics_has_errors())
	{
		r.parse_ok      = false;
		r.error_count   = diagnostics_get_error_count();
		r.warning_count = diagnostics_get_warning_count();
		return r;
	}

	r.parse_ok = true;

	Sema sema       = sema_create(alloc, alloc, &file);
	r.sema_ok       = sema_analyze(&sema, &tu);
	r.error_count   = diagnostics_get_error_count();
	r.warning_count = diagnostics_get_warning_count();
	if (!r.sema_ok)
		return r;

	if (optimize)
	{
		AstOptimizer opt = ast_optimizer_create(alloc, alloc, alloc);
		ast_optimizer_run(&opt, &tu, true);
	}

	BytecodeCompiler compiler = bytecode_compiler_create(alloc, &file);
	Chunk chunk               = bytecode_compiler_compile(&compiler, tu.statements);
	r.chunk                   = chunk;

	VM vm          = vm_create(&chunk, compiler.functions, compiler.function_count);
	r.vm_result    = vm_execute(&vm);
	r.return_value = vm.return_value;

	return r;
}

static TestResults print_test_summary(const char* suite_name, double total_ms)
{
	TestResults results;
	results.run     = g_tests_run;
	results.passed  = g_tests_passed;
	results.failed  = g_tests_failed;
	results.time_ms = total_ms;

	printf("\n  " ANSI_COLOR_DIM "-----------------------------------------------------------------" ANSI_COLOR_RESET
	       "\n");
	if (g_tests_failed == 0)
	{
		printf("  " ANSI_COLOR_GREEN ANSI_COLOR_BOLD "All %d %s tests passed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		       "  (%.2fms total)" ANSI_COLOR_RESET "\n\n",
		       g_tests_run, suite_name, total_ms);
	}
	else
	{
		printf("  " ANSI_COLOR_RED ANSI_COLOR_BOLD "%d of %d %s tests failed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		       "  (%.2fms total)" ANSI_COLOR_RESET "\n",
		       g_tests_failed, g_tests_run, suite_name, total_ms);
		printf("  " ANSI_COLOR_GREEN "%d passed" ANSI_COLOR_RESET ", " ANSI_COLOR_RED "%d failed" ANSI_COLOR_RESET
		       "\n\n",
		       g_tests_passed, g_tests_failed);
	}
	return results;
}
