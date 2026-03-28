#include "ast_optimizer_tests.h"
#include "bytecode_tests.h"
#include "e2e_tests.h"
#include "lexer_tests.h"
#include "parser_tests.h"
#include "sema_tests.h"

int main(void)
{
	PlatformTimer grand_timer;
	platform_timer_start(&grand_timer);

	int total_run     = 0;
	int total_passed  = 0;
	int total_failed  = 0;
	int suites_run    = 0;
	int suites_failed = 0;

	TestResults r;

	r = run_lexer_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	r = run_parser_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	r = run_sema_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	r = run_ast_optimizer_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	r = run_bytecode_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	r = run_e2e_tests();
	total_run += r.run;
	total_passed += r.passed;
	total_failed += r.failed;
	suites_run++;
	if (r.failed > 0)
		suites_failed++;

	double grand_ms = platform_timer_elapsed_ms(&grand_timer);

	printf(ANSI_COLOR_BOLD "=================================================================\n" ANSI_COLOR_RESET);
	if (total_failed == 0)
	{
		printf(ANSI_COLOR_GREEN ANSI_COLOR_BOLD "  All %d tests passed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		                                        " (%d suites, %.2fms total)" ANSI_COLOR_RESET "\n",
		       total_run, suites_run, grand_ms);
	}
	else
	{
		printf(ANSI_COLOR_RED ANSI_COLOR_BOLD "  %d of %d tests failed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		                                      " (%d/%d suites failed, %.2fms total)" ANSI_COLOR_RESET "\n",
		       total_failed, total_run, suites_failed, suites_run, grand_ms);
		printf("  " ANSI_COLOR_GREEN "%d passed" ANSI_COLOR_RESET ", " ANSI_COLOR_RED "%d failed" ANSI_COLOR_RESET "\n",
		       total_passed, total_failed);
	}
	printf(ANSI_COLOR_BOLD "=================================================================\n" ANSI_COLOR_RESET);

	return total_failed > 0 ? 1 : 0;
}
