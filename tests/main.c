#include "lexer_tests.h"
#include "parser_tests.h"
#include "sema_tests.h"

#include "wandelt/defines.h"

int main(void)
{
	bool results_ok = true;

	results_ok = run_lexer_tests();

	if (!results_ok)
	{
		results_ok = run_parser_tests();
	}

	if (!results_ok)
	{
		results_ok = run_sema_tests();
	}

	return 0;
}
