/**
 * @file lexer_tests.c
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#include "wandelt/lexer.h"
#include "wandelt/memory.h"

// ---------------------------------------------------------------------------
// Minimal test framework
// ---------------------------------------------------------------------------

#include <time.h>

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

static void print_section(const char* name)
{
	printf("\n  " ANSI_COLOR_CYAN ANSI_COLOR_BOLD "%s" ANSI_COLOR_RESET "\n", name);
}

static double timespec_diff_ms(struct timespec* start, struct timespec* end)
{
	return (double)(end->tv_sec - start->tv_sec) * 1000.0 +
	       (double)(end->tv_nsec - start->tv_nsec) / 1000000.0;
}

#define TEST(name)                                                                                                    \
	static void test_##name(Allocator* alloc);                                                                        \
	static void run_test_##name(Allocator* alloc)                                                                     \
	{                                                                                                                 \
		g_tests_run++;                                                                                                \
		struct timespec _start, _end;                                                                                 \
		timespec_get(&_start, TIME_UTC);                                                                              \
		printf("  %-50s", #name);                                                                                     \
		test_##name(alloc);                                                                                           \
		timespec_get(&_end, TIME_UTC);                                                                                \
		double _ms = timespec_diff_ms(&_start, &_end);                                                                \
		g_tests_passed++;                                                                                             \
		printf(ANSI_COLOR_GREEN "PASS" ANSI_COLOR_RESET ANSI_COLOR_DIM "  (%.3fms)" ANSI_COLOR_RESET "\n", _ms);      \
	}                                                                                                                 \
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

#define RUN_TEST(name) run_test_##name(&arena)

static File make_file(Allocator* alloc, const char* source)
{
	File f;
	f.alloc        = alloc;
	f.path         = string_from_cstr(alloc, "<test>");
	f.name         = string_from_cstr(alloc, "<test>");
	f.content      = string_from_cstr(alloc, source);
	f.content_size = (u64)strlen(source);

	return f;
}

#define MAX_TOKENS 128

typedef struct TokenList
{
	Token tokens[MAX_TOKENS];
	i32 count;
} TokenList;

static TokenList lex_all(Allocator* alloc, const char* source)
{
	File file   = make_file(alloc, source);
	Lexer lexer = lexer_create(&file);
	TokenList tl;
	tl.count = 0;

	while (tl.count < MAX_TOKENS)
	{
		Token tok             = lexer_peek_token(&lexer);
		tl.tokens[tl.count++] = tok;
		if (tok.type == TOKEN_TYPE_EOF)
			break;
		lexer_eat_token(&lexer);
	}

	return tl;
}

static StringView token_lexeme(const char* src, Token token)
{
	u32 tok_length = token.span.end - token.span.begin;

	return (StringView){.data = src + token.span.begin, .len = tok_length};
}

// --- Empty / whitespace-only inputs ----------------------------------------

TEST(empty_input)
{
	TokenList tl = lex_all(alloc, "");
	ASSERT_EQ(tl.count, 1);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_EOF);
}

TEST(whitespace_only)
{
	TokenList tl = lex_all(alloc, "   \t\n\r  \n  ");
	ASSERT_EQ(tl.count, 1);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_EOF);
}

// --- Single-character tokens -----------------------------------------------

TEST(open_paren)
{
	const char* src = "(";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_OPEN_PAREN);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "(");
	ASSERT_EQ(tl.tokens[1].type, TOKEN_TYPE_EOF);
}

TEST(close_paren)
{
	const char* src = ")";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_CLOSE_PAREN);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ")");
}

TEST(open_brace)
{
	const char* src = "{";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_OPEN_BRACE);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "{");
}

TEST(close_brace)
{
	const char* src = "}";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_CLOSE_BRACE);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "}");
}

TEST(semicolon)
{
	const char* src = ";";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_SEMICOLON);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ";");
}

// --- Identifiers -----------------------------------------------------------

TEST(simple_identifier)
{
	const char* src = "main";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "main");
}

TEST(identifier_with_underscores)
{
	const char* src = "_my_var";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "_my_var");
}

TEST(identifier_with_digits)
{
	const char* src = "x123";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "x123");
}

TEST(single_char_identifier)
{
	const char* src = "x";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "x");
}

TEST(underscore_only_identifier)
{
	const char* src = "_";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "_");
}

TEST(uppercase_identifier)
{
	const char* src = "TOKEN_TYPE_EOF";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "TOKEN_TYPE_EOF");
}

// --- Integers --------------------------------------------------------------

TEST(single_digit)
{
	const char* src = "0";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_INTEGER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "0");
}

TEST(multi_digit_integer)
{
	const char* src = "42";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_INTEGER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "42");
}

TEST(large_integer)
{
	const char* src = "1234567890";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_INTEGER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "1234567890");
}

// --- Peek / eat semantics --------------------------------------------------

TEST(peek_returns_same_token)
{
	const char* src = "abc";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	Token t1 = lexer_peek_token(&lexer);
	Token t2 = lexer_peek_token(&lexer);

	ASSERT_EQ(t1.type, t2.type);
	ASSERT_STR_EQ(token_lexeme(src, t1), "abc");
	ASSERT_STR_EQ(token_lexeme(src, t2), "abc");
}

TEST(eat_then_peek_advances)
{
	const char* src = "a b";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	Token t1 = lexer_peek_token(&lexer);
	ASSERT_STR_EQ(token_lexeme(src, t1), "a");

	lexer_eat_token(&lexer);

	Token t2 = lexer_peek_token(&lexer);
	ASSERT_STR_EQ(token_lexeme(src, t2), "b");
}

TEST(eat_all_reaches_eof)
{
	const char* src = "x y z";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	for (int i = 0; i < 3; i++)
	{
		ASSERT_EQ(lexer_peek_token(&lexer).type != TOKEN_TYPE_EOF, 1);
		lexer_eat_token(&lexer);
	}

	ASSERT_EQ(lexer_peek_token(&lexer).type, TOKEN_TYPE_EOF);
}

// --- Lookahead (peek at offset) --------------------------------------------

TEST(peek_at_offset_zero)
{
	const char* src = "a b c";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	Token t = lexer_peek_token_at_offset(&lexer, 0);
	ASSERT_STR_EQ(token_lexeme(src, t), "a");
}

TEST(peek_at_offset_one)
{
	const char* src = "a b c";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	lexer_peek_token(&lexer);

	Token t = lexer_peek_token_at_offset(&lexer, 1);
	ASSERT_EQ(t.type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, t), "b");
}

TEST(peek_at_offset_two)
{
	const char* src = "a b c";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	lexer_peek_token(&lexer);

	Token t = lexer_peek_token_at_offset(&lexer, 2);
	ASSERT_EQ(t.type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, t), "c");
}

TEST(peek_at_offset_to_eof)
{
	const char* src = "a b";
	File file       = make_file(alloc, src);
	Lexer lexer     = lexer_create(&file);

	lexer_peek_token(&lexer);

	Token t = lexer_peek_token_at_offset(&lexer, 2);
	ASSERT_EQ(t.type, TOKEN_TYPE_EOF);
}

// --- Whitespace variations -------------------------------------------------

TEST(carriage_return_newline)
{
	const char* src = "a\r\nb";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 3);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "a");
	ASSERT_EQ(tl.tokens[1].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[1]), "b");
}

TEST(tokens_separated_by_whitespace)
{
	const char* src = "  foo   123   ;  ";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 4);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_IDENTIFIER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "foo");
	ASSERT_EQ(tl.tokens[1].type, TOKEN_TYPE_INTEGER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[1]), "123");
	ASSERT_EQ(tl.tokens[2].type, TOKEN_TYPE_SEMICOLON);
	ASSERT_EQ(tl.tokens[3].type, TOKEN_TYPE_EOF);
}

int main(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	struct timespec total_start, total_end;
	timespec_get(&total_start, TIME_UTC);

	printf(ANSI_COLOR_BOLD "Running lexer tests..." ANSI_COLOR_RESET "\n");

	print_section("Empty / whitespace");
	RUN_TEST(empty_input);
	RUN_TEST(whitespace_only);

	print_section("Single-character tokens");
	RUN_TEST(open_paren);
	RUN_TEST(close_paren);
	RUN_TEST(open_brace);
	RUN_TEST(close_brace);
	RUN_TEST(semicolon);

	print_section("Identifiers");
	RUN_TEST(simple_identifier);
	RUN_TEST(identifier_with_underscores);
	RUN_TEST(identifier_with_digits);
	RUN_TEST(single_char_identifier);
	RUN_TEST(underscore_only_identifier);
	RUN_TEST(uppercase_identifier);

	print_section("Integers");
	RUN_TEST(single_digit);
	RUN_TEST(multi_digit_integer);
	RUN_TEST(large_integer);

	print_section("Peek / eat semantics");
	RUN_TEST(peek_returns_same_token);
	RUN_TEST(eat_then_peek_advances);
	RUN_TEST(eat_all_reaches_eof);

	print_section("Lookahead (peek at offset)");
	RUN_TEST(peek_at_offset_zero);
	RUN_TEST(peek_at_offset_one);
	RUN_TEST(peek_at_offset_two);
	RUN_TEST(peek_at_offset_to_eof);

	print_section("Whitespace variations");
	RUN_TEST(carriage_return_newline);
	RUN_TEST(tokens_separated_by_whitespace);

	timespec_get(&total_end, TIME_UTC);
	double total_ms = timespec_diff_ms(&total_start, &total_end);

	printf("\n  " ANSI_COLOR_DIM "----------------------------------------" ANSI_COLOR_RESET "\n");

	if (g_tests_failed == 0)
	{
		printf("  " ANSI_COLOR_GREEN ANSI_COLOR_BOLD "All %d tests passed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		       "  (%.2fms total)" ANSI_COLOR_RESET "\n\n",
		       g_tests_run, total_ms);
	}
	else
	{
		printf("  " ANSI_COLOR_RED ANSI_COLOR_BOLD "%d of %d tests failed" ANSI_COLOR_RESET ANSI_COLOR_DIM
		       "  (%.2fms total)" ANSI_COLOR_RESET "\n",
		       g_tests_failed, g_tests_run, total_ms);
		printf("  " ANSI_COLOR_GREEN "%d passed" ANSI_COLOR_RESET ", " ANSI_COLOR_RED "%d failed" ANSI_COLOR_RESET
		       "\n\n",
		       g_tests_passed, g_tests_failed);
	}

	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return g_tests_failed > 0 ? 1 : 0;
}
