#include "wandelt/diagnostics.h"
#include "wandelt/memory.h"
#include "wandelt/parser.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"

int main(void)
{
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
	if (diagnostics_has_errors())
	{
		printf("Compilation failed with %d error(s) and %d warning(s)\n", diagnostics_get_error_count(),
		       diagnostics_get_warning_count());
		return 1;
	}

	ast_dump_statements(tu.statements);

	string_arena.release(string_arena.ctx);
	stmt_arena.release(stmt_arena.ctx);
	decl_arena.release(decl_arena.ctx);
	expr_arena.release(expr_arena.ctx);
	heap.release(heap.ctx);

	return 0;
}
