#include "parser_tests.h"

#include "test_framework.h"

static TranslationUnit parse_source(Allocator* alloc, const char* source)
{
	File file          = make_test_file(alloc, source);
	Lexer lexer        = lexer_create(&file);
	Parser parser      = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu = parser_parse(&parser);

	return tu;
}

// ---------------------------------------------------------------------------
// Operator precedence
// ---------------------------------------------------------------------------

TEST(precedence_mul_before_add)
{
	TranslationUnit tu = parse_source(alloc, "return 2 + 3 * 4;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_ADD);
	ASSERT_EQ(expr->binary.left->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(expr->binary.left->constant.integer_value, 2);

	Expression* right = expr->binary.right;
	ASSERT_EQ(right->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(right->binary.operator, BINARY_OPERATOR_MUL);
	ASSERT_EQ(right->binary.left->constant.integer_value, 3);
	ASSERT_EQ(right->binary.right->constant.integer_value, 4);
}

TEST(precedence_add_before_comparison)
{
	TranslationUnit tu = parse_source(alloc, "return 2 == 3 + 4;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_EQ);
	ASSERT_EQ(expr->binary.left->constant.integer_value, 2);

	Expression* right = expr->binary.right;
	ASSERT_EQ(right->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(right->binary.operator, BINARY_OPERATOR_ADD);
}

TEST(precedence_left_associative_subtraction)
{
	TranslationUnit tu = parse_source(alloc, "return 10 - 3 - 2;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_SUB);

	Expression* left = expr->binary.left;
	ASSERT_EQ(left->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(left->binary.operator, BINARY_OPERATOR_SUB);
	ASSERT_EQ(left->binary.left->constant.integer_value, 10);
	ASSERT_EQ(left->binary.right->constant.integer_value, 3);
	ASSERT_EQ(expr->binary.right->constant.integer_value, 2);
}

TEST(precedence_parens_override)
{
	TranslationUnit tu = parse_source(alloc, "return (2 + 3) * 4;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_MUL);

	Expression* group = expr->binary.left;
	ASSERT_EQ(group->type, EXPRESSION_TYPE_GROUP);
	ASSERT_EQ(group->group.inner->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(group->group.inner->binary.operator, BINARY_OPERATOR_ADD);
}

TEST(precedence_unary_binds_tight)
{
	TranslationUnit tu = parse_source(alloc, "return -2 + 3;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_ADD);

	Expression* left = expr->binary.left;
	ASSERT_EQ(left->type, EXPRESSION_TYPE_UNARY);
	ASSERT_EQ(left->unary.operator, UNARY_OPERATOR_NEGATE);
	ASSERT_EQ(left->unary.operand->constant.integer_value, 2);
}

TEST(precedence_cast_highest_infix)
{
	TranslationUnit tu = parse_source(alloc, "return 2 + 3 as float;");
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_RETURN);

	Expression* expr = tu.statements[0]->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
	ASSERT_EQ(expr->binary.operator, BINARY_OPERATOR_ADD);

	Expression* right = expr->binary.right;
	ASSERT_EQ(right->type, EXPRESSION_TYPE_CAST);
	ASSERT_EQ(right->cast.target_type->kind, TYPE_KIND_FLOAT);
	ASSERT_EQ(right->cast.expression->constant.integer_value, 3);
}

// ---------------------------------------------------------------------------
// Declaration Parsing
// ---------------------------------------------------------------------------

TEST(parse_var_int_declaration)
{
	TranslationUnit tu = parse_source(alloc, "var int x = 42;");
	ASSERT_EQ(vector_get_length(tu.statements), 1);

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_DECLARATION);

	Declaration* decl = stmt->decl_stmt.declaration;
	ASSERT_EQ(decl->type, DECLARATION_TYPE_VARIABLE);
	ASSERT_STR_EQ(decl->variable.name, "x");
	ASSERT_EQ(decl->variable.type->kind, TYPE_KIND_INT);

	Expression* init = decl->variable.initializer;
	ASSERT_EQ(init->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_INTEGER);
	ASSERT_EQ(init->constant.integer_value, 42);
}

TEST(parse_var_float_declaration)
{
	TranslationUnit tu = parse_source(alloc, "var float f = 3.14f;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_DECLARATION);

	Declaration* decl = stmt->decl_stmt.declaration;
	ASSERT_EQ(decl->variable.type->kind, TYPE_KIND_FLOAT);

	Expression* init = decl->variable.initializer;
	ASSERT_EQ(init->constant.kind, CONSTANT_KIND_FLOAT);
	ASSERT_FLOAT_EQ(init->constant.float_value, 3.14f, 0.001f);
}

TEST(parse_var_bool_true)
{
	TranslationUnit tu = parse_source(alloc, "var bool b = true;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_DECLARATION);

	Declaration* decl = stmt->decl_stmt.declaration;
	ASSERT_EQ(decl->variable.type->kind, TYPE_KIND_BOOL);
	ASSERT_EQ(decl->variable.initializer->constant.kind, CONSTANT_KIND_BOOLEAN);
	ASSERT_TRUE(decl->variable.initializer->constant.boolean_value);
}

TEST(parse_namespace_declaration)
{
	TranslationUnit tu = parse_source(alloc, "namespace myns;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_DECLARATION);

	Declaration* decl = stmt->decl_stmt.declaration;
	ASSERT_EQ(decl->type, DECLARATION_TYPE_NAMESPACE);
	ASSERT_STR_EQ(decl->namespace.name, "myns");
}

TEST(parse_return_statement)
{
	TranslationUnit tu = parse_source(alloc, "return 42;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	ASSERT_EQ(stmt->return_stmt.expression->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(stmt->return_stmt.expression->constant.integer_value, 42);
}

// ---------------------------------------------------------------------------
// All type keywords parse correctly
// ---------------------------------------------------------------------------

TEST(parse_all_type_keywords)
{
	struct
	{
		const char* src;
		TypeKind expected;
	} cases[] = {
	    {"var bool x = true;", TYPE_KIND_BOOL},     {"var char x = 1;", TYPE_KIND_CHAR},
	    {"var uchar x = 1;", TYPE_KIND_UCHAR},      {"var short x = 1;", TYPE_KIND_SHORT},
	    {"var ushort x = 1;", TYPE_KIND_USHORT},    {"var int x = 1;", TYPE_KIND_INT},
	    {"var uint x = 1;", TYPE_KIND_UINT},        {"var long x = 1;", TYPE_KIND_LONG},
	    {"var ulong x = 1;", TYPE_KIND_ULONG},      {"var float x = 1.0f;", TYPE_KIND_FLOAT},
	    {"var double x = 1.0d;", TYPE_KIND_DOUBLE},
	};

	for (int i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++)
	{
		TranslationUnit tu = parse_source(alloc, cases[i].src);

		Statement* stmt = tu.statements[0];
		ASSERT_EQ(stmt->type, STATEMENT_TYPE_DECLARATION);

		Declaration* decl = stmt->decl_stmt.declaration;
		ASSERT_EQ(decl->variable.type->kind, cases[i].expected);
	}
}

// ---------------------------------------------------------------------------
// All comparison operators
// ---------------------------------------------------------------------------

TEST(parse_all_comparison_operators)
{
	struct
	{
		const char* src;
		BinaryOperator expected;
	} cases[] = {
	    {"return 1 == 2;", BINARY_OPERATOR_EQ},  {"return 1 != 2;", BINARY_OPERATOR_NEQ},
	    {"return 1 < 2;", BINARY_OPERATOR_LT},   {"return 1 > 2;", BINARY_OPERATOR_GT},
	    {"return 1 <= 2;", BINARY_OPERATOR_LEQ}, {"return 1 >= 2;", BINARY_OPERATOR_GEQ},
	};

	for (int i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++)
	{
		TranslationUnit tu = parse_source(alloc, cases[i].src);

		Statement* stmt = tu.statements[0];
		ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

		Expression* expr = stmt->return_stmt.expression;
		ASSERT_NOT_NULL(expr);
		ASSERT_EQ(expr->type, EXPRESSION_TYPE_BINARY);
		ASSERT_EQ(expr->binary.operator, cases[i].expected);
	}
}

// ---------------------------------------------------------------------------
// Cast expression parsing
// ---------------------------------------------------------------------------
TEST(parse_cast_expression)
{
	TranslationUnit tu = parse_source(alloc, "return 5 as float;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* expr = stmt->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_CAST);
	ASSERT_EQ(expr->cast.target_type->kind, TYPE_KIND_FLOAT);
	ASSERT_EQ(expr->cast.expression->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(expr->cast.expression->constant.integer_value, 5);
}

// ---------------------------------------------------------------------------
// Unary negation
// ---------------------------------------------------------------------------

TEST(parse_unary_negation)
{
	TranslationUnit tu = parse_source(alloc, "return -42;");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* expr = stmt->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_UNARY);
	ASSERT_EQ(expr->unary.operator, UNARY_OPERATOR_NEGATE);
	ASSERT_EQ(expr->unary.operand->type, EXPRESSION_TYPE_CONSTANT);
	ASSERT_EQ(expr->unary.operand->constant.integer_value, 42);
}

TEST(parse_double_negation)
{
	TranslationUnit tu = parse_source(alloc, "return -(-5);");

	Statement* stmt = tu.statements[0];
	ASSERT_EQ(stmt->type, STATEMENT_TYPE_RETURN);

	Expression* expr = stmt->return_stmt.expression;
	ASSERT_EQ(expr->type, EXPRESSION_TYPE_UNARY);
	ASSERT_EQ(expr->unary.operator, UNARY_OPERATOR_NEGATE);

	Expression* inner = expr->unary.operand;
	ASSERT_EQ(inner->type, EXPRESSION_TYPE_GROUP);

	Expression* inner_unary = inner->group.inner;
	ASSERT_EQ(inner_unary->type, EXPRESSION_TYPE_UNARY);
	ASSERT_EQ(inner_unary->unary.operator, UNARY_OPERATOR_NEGATE);
	ASSERT_EQ(inner_unary->unary.operand->constant.integer_value, 5);
}

// ---------------------------------------------------------------------------
// Multiple statements
// ---------------------------------------------------------------------------

TEST(parse_multiple_statements)
{
	const char* src = "var int x = 5;\n"
	                  "var int y = 10;\n"
	                  "return x;";

	TranslationUnit tu = parse_source(alloc, src);
	ASSERT_EQ(vector_get_length(tu.statements), 3);
	ASSERT_EQ(tu.statements[0]->type, STATEMENT_TYPE_DECLARATION);
	ASSERT_EQ(tu.statements[1]->type, STATEMENT_TYPE_DECLARATION);
	ASSERT_EQ(tu.statements[2]->type, STATEMENT_TYPE_RETURN);
}

TestResults run_parser_tests(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running parser tests..." ANSI_COLOR_RESET "\n");

	print_section("Precedence & Associativity");
	RUN_TEST(precedence_mul_before_add);
	RUN_TEST(precedence_add_before_comparison);
	RUN_TEST(precedence_left_associative_subtraction);
	RUN_TEST(precedence_parens_override);
	RUN_TEST(precedence_unary_binds_tight);
	RUN_TEST(precedence_cast_highest_infix);

	print_section("Declaration Parsing");
	RUN_TEST(parse_var_int_declaration);
	RUN_TEST(parse_var_float_declaration);
	RUN_TEST(parse_var_bool_true);
	RUN_TEST(parse_namespace_declaration);
	RUN_TEST(parse_return_statement);

	print_section("All type keywords");
	RUN_TEST(parse_all_type_keywords);

	print_section("All comparison operators");
	RUN_TEST(parse_all_comparison_operators);

	print_section("Cast expression parsing");
	RUN_TEST(parse_cast_expression);

	print_section("Unary negation");
	RUN_TEST(parse_unary_negation);
	RUN_TEST(parse_double_negation);

	print_section("Multiple statements");
	RUN_TEST(parse_multiple_statements);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("parser", total_ms);
}
