#include "wandelt/ast.h"
#include "wandelt/ast_opt.h"
#include "wandelt/bytecode.h"
#include "wandelt/defines.h"
#include "wandelt/diagnostics.h"
#include "wandelt/disassembler.h"
#include "wandelt/file.h"
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/parser.h"
#include "wandelt/platform.h"
#include "wandelt/sema.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"
#include "wandelt/vm.h"

int main(int argc, char* argv[])
{
	setvbuf(stdout, NULL, _IOFBF, 8192);

	bool debug    = false;
	bool optimize = false;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-debug") == 0)
			debug = true;
		if (strcmp(argv[i], "-o") == 0)
			optimize = true;
	}

	Allocator heap         = allocator_get_heap_allocator();
	Allocator stmt_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator decl_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator expr_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator string_arena = allocator_get_arena_allocator(&heap, MB(2));

	PlatformTimer timer;
	double dt_lexing_parsing = 0.0;
	double dt_sema           = 0.0;
	double dt_ast_opt        = 0.0;
	double dt_bytecode       = 0.0;
	double dt_vm             = 0.0;

	String demo_filepath = string_from_cstr(&string_arena, DEMO_PATH "main.wdt");
	File demo_file       = file_create(&string_arena, demo_filepath);

	// Lexing & Parsing
	platform_timer_start(&timer);
	Lexer lexer        = lexer_create(&demo_file);
	Parser parser      = parser_create(&stmt_arena, &decl_arena, &expr_arena, &lexer);
	TranslationUnit tu = parser_parse(&parser);
	dt_lexing_parsing  = platform_timer_elapsed_ms(&timer);

	if (diagnostics_has_errors())
	{
		printf("Compilation failed with %d error(s) and %d warning(s)\n", diagnostics_get_error_count(),
		       diagnostics_get_warning_count());
		return 1;
	}

	// Semantic Analysis
	platform_timer_start(&timer);
	Sema sema    = sema_create(&expr_arena, &decl_arena, &demo_file);
	bool sema_ok = sema_analyze(&sema, &tu);
	dt_sema      = platform_timer_elapsed_ms(&timer);

	if (!sema_ok)
	{
		printf("Compilation failed with %d error(s) and %d warning(s)\n", diagnostics_get_error_count(),
		       diagnostics_get_warning_count());
		return 1;
	}

	if (debug)
	{
		printf("======== Before optimization: =======\n");
		ast_dump_statements(tu.statements);
	}

	// AST Optimization
	if (optimize)
	{
		platform_timer_start(&timer);
		AstOptimizer optimizer = ast_optimizer_create(&expr_arena);
		ast_optimizer_run(&optimizer, &tu);
		dt_ast_opt = platform_timer_elapsed_ms(&timer);

		if (debug)
		{
			printf("======== After optimization: =======\n");
			ast_dump_statements(tu.statements);
		}
	}

	// Bytecode Generation
	Allocator bytecode_arena = allocator_get_arena_allocator(&heap, MB(4));

	platform_timer_start(&timer);
	BytecodeCompiler compiler = bytecode_compiler_create(&bytecode_arena, &demo_file);
	Chunk chunk               = bytecode_compiler_compile(&compiler, tu.statements);
	dt_bytecode               = platform_timer_elapsed_ms(&timer);

	if (debug)
		disassemble_chunk(&chunk, "main", &demo_file);
	disassemble_chunk_to_file(&chunk, "main", &demo_file, "demo/main.wdtbc");

	// VM Execution
	VM vm           = vm_create(&chunk);
	VmResult result = VM_ERROR;

	platform_timer_start(&timer);
	result = vm_execute(&vm);
	dt_vm  = platform_timer_elapsed_ms(&timer);

	// Timing table
	double dt_total = dt_lexing_parsing + dt_sema + dt_ast_opt + dt_bytecode + dt_vm;

	printf("\n");
	printf("Lexing & Parsing:      %.3fms\n", dt_lexing_parsing);
	printf("Semantic Analysis:     %.3fms\n", dt_sema);
	printf("AST Optimization:      %.3fms\n", dt_ast_opt);
	printf("Bytecode Generation:   %.3fms\n", dt_bytecode);
	printf("VM Execution:          %.3fms\n", dt_vm);
	printf("Total:                 %.3fms\n", dt_total);
	printf("\n");

	if (result == VM_OK)
	{
		printf("Program returned: %lld\n", vm.return_value.integer);
	}
	else
	{
		printf("Runtime error occurred!\n");
	}

	bytecode_arena.release(bytecode_arena.ctx);

	string_arena.release(string_arena.ctx);
	stmt_arena.release(stmt_arena.ctx);
	decl_arena.release(decl_arena.ctx);
	expr_arena.release(expr_arena.ctx);
	heap.release(heap.ctx);

	return 0;
}
