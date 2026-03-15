#include "wandelt/file.h"
#include "wandelt/lexer.h"
#include "wandelt/memory.h"
#include "wandelt/string.h"

int main(void)
{
	Allocator heap  = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, MB(12));

	String demo_filepath = string_from_cstr(&arena, DEMO_PATH "main.wdt");
	File demo_file       = file_create(&arena, demo_filepath);
	file_print_info(&demo_file);

	Lexer lexer = lexer_create(&demo_file);

	while (lexer_peek_token(&lexer).type != TOKEN_TYPE_EOF)
	{
		const Token tok = lexer_peek_token(&lexer);
		lexer_eat_token(&lexer);

		printf("<Parsed token: \"%.*s\" at %.*s:%u:%u />\n", FMT_STR_ARG(tok.lexeme),
		       FMT_STR_ARG(tok.source_location.filename), tok.source_location.start_row, tok.source_location.start_col);
	}

	printf("Lexing finished successfully!");

	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return 0;
}
