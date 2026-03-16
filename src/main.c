#include "wandelt/file.h"
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/string.h"

int main(void)
{
	Allocator heap         = allocator_get_heap_allocator();
	Allocator string_arena = allocator_get_arena_allocator(&heap, MB(12));

	String demo_filepath = string_from_cstr(&string_arena, DEMO_PATH "main.wdt");
	File demo_file       = file_create(&string_arena, demo_filepath);
	file_print_info(&demo_file);

	Lexer lexer = lexer_create(&demo_file);

	while (lexer_peek_token(&lexer).type != TOKEN_TYPE_EOF)
	{
		const Token tok = lexer_peek_token(&lexer);
		lexer_eat_token(&lexer);

		lexer_debug_print_token(&lexer, tok);
	}

	printf("Lexing finished successfully!\n");

	string_arena.release(string_arena.ctx);
	heap.release(heap.ctx);

	return 0;
}
