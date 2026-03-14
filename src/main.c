#include "wandelt/memory.h"
#include "wandelt/string.h"
#include "wandelt/file.h"

int main(void)
{
	Allocator heap = allocator_get_heap_allocator();
	Allocator arena = allocator_get_arena_allocator(&heap, 1024);

	String demo_filepath = string_from_cstr(&arena, DEMO_PATH "main.wdt");
	File demo_file = file_create(&arena, demo_filepath);
	file_print_info(&demo_file);

	String s = string_from_cstr(&arena, "Hello there my friend!\n");

	printf("%.*s", FMT_STR_ARG(s));
	printf("%.*s", FMT_STR_ARG(demo_file.content));

	arena.release(arena.ctx);
	heap.release(heap.ctx);

	return 0;
}
