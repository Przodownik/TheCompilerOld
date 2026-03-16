#include "memory.h"
#include "defines.h"

static void* heap_alloc(void* ctx, u64 size)
{
	(void)ctx;

	void* memory = calloc(1, size);
	ASSERT(memory != nullptr, "Failed to allocate memory!");

	return memory;
}

static void* heap_realloc(void* ctx, void* ptr, u64 old_size, u64 new_size)
{
	(void)ctx;
	(void)old_size;

	void* memory = realloc(ptr, new_size);
	ASSERT(memory != nullptr, "Failed to reallocate memory!");

	return memory;
}

static void heap_free(void* ctx, void* ptr, u64 size)
{
	(void)ctx;
	(void)size;

	free(ptr);
}

static void heap_release(void* ctx)
{
	(void)ctx;
}

static void heap_reset(void* ctx)
{
	(void)ctx;
}

static Allocator g_heap_allocator = {
    .alloc   = heap_alloc,
    .realloc = heap_realloc,
    .free    = heap_free,
    .release = heap_release,
    .reset   = heap_reset,
    .ctx     = nullptr,
};

Allocator allocator_get_heap_allocator()
{
	return g_heap_allocator;
}

static void* arena_allocate(void* ctx, u64 size)
{
	ASSERT(size > 0, "Cannot allocate <0 bytes");
	ASSERT(ctx != nullptr, "Context is nullptr");

	ArenaAllocator* arena = (ArenaAllocator*)ctx;

	size = (size + 15u) & ~(u64)15; // round up to 16 bytes

	ASSERT(arena->used + size <= arena->size, "Arena allocator' is out of memory");

	void* ptr = (u8*)arena->memory + arena->used;

	arena->used += size;
	arena->allocations++;

	return ptr;
}

static void* arena_realloc(void* ctx, void* ptr, u64 old_size, u64 new_size)
{
	ASSERT(ctx != nullptr, "Context is nullptr");
	ArenaAllocator* arena = (ArenaAllocator*)ctx;

	void* new_ptr = arena_allocate(arena, new_size);
	if (new_ptr != nullptr && ptr != nullptr)
	{
		// Copy old data to new location
		u64 copy_size = old_size < new_size ? old_size : new_size;
		memcpy(new_ptr, ptr, copy_size);
	}

	return new_ptr;
}

static void arena_free(void* ctx, void* ptr, u64 size)
{
	(void)ctx;
	(void)size;
	(void)ptr;
}

static void arena_reset(void* ctx)
{
	ASSERT(ctx != nullptr, "Context is nullptr");

	ArenaAllocator* arena = (ArenaAllocator*)ctx;

	arena->used        = 0u;
	arena->allocations = 0u;
}

static void arena_release(void* ctx)
{
	ASSERT(ctx != nullptr, "Context is nullptr");

	ArenaAllocator* arena = (ArenaAllocator*)ctx;
	Allocator* base       = arena->base_allocator;

	base->free(base->ctx, arena->memory, arena->size);
	base->free(base->ctx, arena, sizeof(ArenaAllocator));
}

Allocator allocator_get_arena_allocator(Allocator* base_allocator, u64 size)
{
	ASSERT(base_allocator != nullptr, "Allocator is nullptr!");
	ASSERT(size > 0, "Cannot create allocator with %llu size!", size);
	ASSERT(size % 16 == 0, "Allocator size must be a multiple of 16 bytes!");

	ArenaAllocator* arena = base_allocator->alloc(base_allocator->ctx, sizeof(ArenaAllocator));

	arena->base_allocator = base_allocator;
	arena->memory         = base_allocator->alloc(base_allocator->ctx, size);
	arena->size           = size;
	arena->used           = 0u;
	arena->allocations    = 0u;

	Allocator allocator = (Allocator){
	    .alloc   = arena_allocate,
	    .realloc = arena_realloc,
	    .free    = arena_free,
	    .release = arena_release,
	    .reset   = arena_reset,
	    .ctx     = arena,
	};

	return allocator;
}
