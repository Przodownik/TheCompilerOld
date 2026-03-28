#include "bytecode_tests.h"

#include "test_framework.h"

typedef struct BytecodeTestResult
{
	Chunk chunk;
	bool ok;
} BytecodeTestResult;

static BytecodeTestResult compile_to_bytecode(Allocator* alloc, const char* source)
{
	BytecodeTestResult r = {0};
	File file            = make_test_file(alloc, source);
	Lexer lexer          = lexer_create(&file);
	Parser parser        = parser_create(alloc, alloc, alloc, &lexer);
	TranslationUnit tu   = parser_parse(&parser);

	if (diagnostics_has_errors())
	{
		r.ok = false;
		return r;
	}
	Sema sema = sema_create(alloc, alloc, &file);
	if (!sema_analyze(&sema, &tu))
	{
		r.ok = false;
		return r;
	}

	BytecodeCompiler compiler = bytecode_compiler_create(alloc, &file);
	r.chunk                   = bytecode_compiler_compile(&compiler, tu.statements);
	r.ok                      = true;

	return r;
}

static OpCode instr_op(Chunk* chunk, u32 index)
{
	return (OpCode)DECODE_OP(chunk->instructions[index]);
}

// ---------------------------------------------------------------------------
// Bytecode generation tests
// ---------------------------------------------------------------------------

TEST(bytecode_return_constant)
{
	// "return 42;" =>
	//   LOAD_CONST R0, K0    ; K0 = 42
	//   RETURN     R0
	//   HALT
	BytecodeTestResult r = compile_to_bytecode(alloc, "return 42;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 0), OP_CODE_LOAD_CONST);
	ASSERT_EQ(instr_op(&r.chunk, 1), OP_CODE_RETURN);
	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_HALT);

	ASSERT_EQ(vector_get_length(r.chunk.constants), 1);
	ASSERT_EQ(r.chunk.constants[0].i64_val, 42);
}

TEST(bytecode_add_two_constants)
{
	// "return 2 + 3;" =>
	//   LOAD_CONST R0, K0    ; 2
	//   LOAD_CONST R1, K1    ; 3
	//   ADD_I      R0, R0, R1
	//   RETURN     R0
	//   HALT
	BytecodeTestResult r = compile_to_bytecode(alloc, "return 2 + 3;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 0), OP_CODE_LOAD_CONST);
	ASSERT_EQ(instr_op(&r.chunk, 1), OP_CODE_LOAD_CONST);
	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_ADD_I);
	ASSERT_EQ(instr_op(&r.chunk, 3), OP_CODE_RETURN);

	Instruction add = r.chunk.instructions[2];
	ASSERT_EQ(DECODE_A(add), 0); // dest
	ASSERT_EQ(DECODE_B(add), 0); // left
	ASSERT_EQ(DECODE_C(add), 1); // right
}

TEST(bytecode_float_add)
{
	BytecodeTestResult r = compile_to_bytecode(alloc, "return 1.5f + 2.5f;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_ADD_F);
}

TEST(bytecode_double_mul)
{
	BytecodeTestResult r = compile_to_bytecode(alloc, "return 2.0d * 3.0d;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_MUL_D);
}

TEST(bytecode_comparison_gt)
{
	BytecodeTestResult r = compile_to_bytecode(alloc, "return 3 > 2;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_GT_I);
}

TEST(bytecode_negation)
{
	BytecodeTestResult r = compile_to_bytecode(alloc, "return -5;");
	ASSERT_TRUE(r.ok);

	ASSERT_EQ(instr_op(&r.chunk, 0), OP_CODE_LOAD_CONST);
	ASSERT_EQ(instr_op(&r.chunk, 1), OP_CODE_NEG_I);
	ASSERT_EQ(instr_op(&r.chunk, 2), OP_CODE_RETURN);
}

TestResults run_bytecode_tests(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(4));

	PlatformTimer total_timer;
	platform_timer_start(&total_timer);

	printf(ANSI_COLOR_BOLD "Running bytecode tests..." ANSI_COLOR_RESET "\n");

	print_section("Bytecode generation");
	RUN_TEST(bytecode_return_constant);
	RUN_TEST(bytecode_add_two_constants);
	RUN_TEST(bytecode_float_add);
	RUN_TEST(bytecode_double_mul);
	RUN_TEST(bytecode_comparison_gt);
	RUN_TEST(bytecode_negation);

	double total_ms = platform_timer_elapsed_ms(&total_timer);
	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return print_test_summary("bytecode", total_ms);
}
