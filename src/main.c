#include "wandelt/ast.h"
#include "wandelt/bytecode.h"
#include "wandelt/defines.h"
#include "wandelt/diagnostics.h"
#include "wandelt/disassembler.h"
#include "wandelt/file.h"
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/parser.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"
#include "wandelt/vm.h"

int main(int argc, char* argv[])
{
	bool debug = false;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-debug") == 0)
		{
			debug = true;
		}
	}

	Allocator heap         = allocator_get_heap_allocator();
	Allocator stmt_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator decl_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator expr_arena   = allocator_get_arena_allocator(&heap, MB(2));
	Allocator string_arena = allocator_get_arena_allocator(&heap, MB(2));

	String demo_filepath = string_from_cstr(&string_arena, DEMO_PATH "main.wdt");
	File demo_file       = file_create(&string_arena, demo_filepath);

	Lexer lexer   = lexer_create(&demo_file);
	Parser parser = parser_create(&stmt_arena, &decl_arena, &expr_arena, &lexer);

	TranslationUnit tu = parser_parse(&parser);
	if (debug)
		ast_dump_statements(tu.statements);
	if (diagnostics_has_errors())
	{
		printf("Compilation failed with %d error(s) and %d warning(s)\n", diagnostics_get_error_count(),
		       diagnostics_get_warning_count());
		return 1;
	}

	// for now remove the namespace decl stmt;
	Statement* stmt = nullptr;
	vector_remove_at(tu.statements, 0, &stmt);

	Allocator bytecode_arena = allocator_get_arena_allocator(&heap, MB(4));

	BytecodeCompiler compiler = bytecode_compiler_create(&bytecode_arena, &demo_file);
	Chunk chunk               = bytecode_compiler_compile(&compiler, tu.statements);
	if (debug)
		disassemble_chunk(&chunk, "main", &demo_file);
	disassemble_chunk_to_file(&chunk, "main", &demo_file, "demo/main.wdtbc");
	VM vm           = vm_create(&chunk);
	VmResult result = vm_execute(&vm);
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
