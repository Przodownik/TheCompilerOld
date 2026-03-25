/**
 * @file lexer_tests.c
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/platform.h"

// ---------------------------------------------------------------------------
// Minimal test framework
// ---------------------------------------------------------------------------

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

static_assert(TOKEN_TYPE_COUNT == 41, "Update lexer tests when adding new token types");

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

TEST(comments_only)
{
	const char* src = "// This is a comment\n// Another comment\n";
	TokenList tl    = lex_all(alloc, src);
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

TEST(plus)
{
	const char* src = "+";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_PLUS);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "+");
}

TEST(minus)
{
	const char* src = "-";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_MINUS);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "-");
}

TEST(star)
{
	const char* src = "*";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_STAR);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "*");
}

TEST(slash)
{
	const char* src = "/";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_SLASH);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "/");
}

TEST(equals)
{
	const char* src = "=";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_EQUALS);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "=");
}

TEST(dot)
{
	const char* src = ".";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_DOT);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ".");
}

TEST(greater)
{
	const char* src = ">";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_GREATER);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ">");
}

TEST(less)
{
	const char* src = "<";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_LESS);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "<");
}

// --- Double-character tokens -----------------------------------------------

TEST(greater_equal)
{
	const char* src = ">=";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_GREATER_EQUAL);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ">=");
}

TEST(less_equal)
{
	const char* src = "<=";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_LESS_EQUAL);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "<=");
}

TEST(equal_equal)
{
	const char* src = "==";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_EQUAL_EQUAL);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "==");
}

TEST(bang_equal)
{
	const char* src = "!=";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_BANG_EQUAL);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "!=");
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

// --- Keywords --------------------------------------------------------------

TEST(function_keyword)
{
	const char* src = "fn";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FUNCTION_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "fn");
}

TEST(return_keyword)
{
	const char* src = "return";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_RETURN_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "return");
}

TEST(namespace_keyword)
{
	const char* src = "namespace";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_NAMESPACE_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "namespace");
}

TEST(var_keyword)
{
	const char* src = "var";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_VAR_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "var");
}

TEST(as_keyword)
{
	const char* src = "as";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_AS_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "as");
}

TEST(bool_keyword)
{
	const char* src = "bool";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_BOOL_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "bool");
}

TEST(char_keyword)
{
	const char* src = "char";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_CHAR_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "char");
}

TEST(uchar_keyword)
{
	const char* src = "uchar";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_UCHAR_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "uchar");
}

TEST(short_keyword)
{
	const char* src = "short";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_SHORT_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "short");
}

TEST(ushort_keyword)
{
	const char* src = "ushort";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_USHORT_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "ushort");
}

TEST(int_keyword)
{
	const char* src = "int";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_INT_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "int");
}

TEST(uint_keyword)
{
	const char* src = "uint";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_UINT_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "uint");
}

TEST(long_keyword)
{
	const char* src = "long";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_LONG_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "long");
}

TEST(ulong_keyword)
{
	const char* src = "ulong";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_ULONG_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "ulong");
}

TEST(float_keyword)
{
	const char* src = "float";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FLOAT_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "float");
}

TEST(double_keyword)
{
	const char* src = "double";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_DOUBLE_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "double");
}

TEST(true_keyword)
{
	const char* src = "true";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_TRUE_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "true");
}

TEST(false_keyword)
{
	const char* src = "false";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FALSE_KEYWORD);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "false");
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

// --- Floats and doubles ----------------------------------------------------

TEST(float_literal_1)
{
	const char* src = "3.14f";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FLOAT);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "3.14f");
}

TEST(float_literal_2)
{
	const char* src = "3.f";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FLOAT);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "3.f");
}

TEST(float_literal_3)
{
	const char* src = ".14f";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_FLOAT);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ".14f");
}

TEST(double_literal_1)
{
	const char* src = "2.71828d";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_DOUBLE);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "2.71828d");
}

TEST(double_literal_2)
{
	const char* src = "2.d";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_DOUBLE);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), "2.d");
}

TEST(double_literal_3)
{
	const char* src = ".71828d";
	TokenList tl    = lex_all(alloc, src);
	ASSERT_EQ(tl.count, 2);
	ASSERT_EQ(tl.tokens[0].type, TOKEN_TYPE_DOUBLE);
	ASSERT_STR_EQ(token_lexeme(src, tl.tokens[0]), ".71828d");
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

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running lexer tests..." ANSI_COLOR_RESET "\n");

	print_section("Empty / whitespace");
	RUN_TEST(empty_input);
	RUN_TEST(whitespace_only);
	RUN_TEST(comments_only);

	print_section("Single-character tokens");
	RUN_TEST(open_paren);
	RUN_TEST(close_paren);
	RUN_TEST(open_brace);
	RUN_TEST(close_brace);
	RUN_TEST(semicolon);
	RUN_TEST(plus);
	RUN_TEST(minus);
	RUN_TEST(star);
	RUN_TEST(slash);
	RUN_TEST(equals);
	RUN_TEST(dot);
	RUN_TEST(greater);
	RUN_TEST(less);

	print_section("Double-character tokens");
	RUN_TEST(greater_equal);
	RUN_TEST(less_equal);
	RUN_TEST(equal_equal);
	RUN_TEST(bang_equal);

	print_section("Identifiers");
	RUN_TEST(simple_identifier);
	RUN_TEST(identifier_with_underscores);
	RUN_TEST(identifier_with_digits);
	RUN_TEST(single_char_identifier);
	RUN_TEST(underscore_only_identifier);
	RUN_TEST(uppercase_identifier);

	print_section("Keywords");
	RUN_TEST(function_keyword);
	RUN_TEST(return_keyword);
	RUN_TEST(namespace_keyword);
	RUN_TEST(var_keyword);
	RUN_TEST(as_keyword);

	RUN_TEST(bool_keyword);
	RUN_TEST(char_keyword);
	RUN_TEST(uchar_keyword);
	RUN_TEST(short_keyword);
	RUN_TEST(ushort_keyword);
	RUN_TEST(int_keyword);
	RUN_TEST(uint_keyword);
	RUN_TEST(long_keyword);
	RUN_TEST(ulong_keyword);
	RUN_TEST(float_keyword);
	RUN_TEST(double_keyword);
	RUN_TEST(true_keyword);
	RUN_TEST(false_keyword);

	print_section("Integers");
	RUN_TEST(single_digit);
	RUN_TEST(multi_digit_integer);
	RUN_TEST(large_integer);

	print_section("Floats and doubles");
	RUN_TEST(float_literal_1);
	RUN_TEST(float_literal_2);
	RUN_TEST(float_literal_3);
	RUN_TEST(double_literal_1);
	RUN_TEST(double_literal_2);
	RUN_TEST(double_literal_3);

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

	double total_ms = platform_timer_elapsed_ms(&total_timer);

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
