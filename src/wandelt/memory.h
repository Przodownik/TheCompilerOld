/**
 * @file memory.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"

typedef void* (*alloc_fn)(void* ctx, u64 size);
typedef void* (*realloc_fn)(void* ctx, void* ptr, u64 old_size, u64 new_size);
typedef void (*free_fn)(void* ctx, void* ptr, u64 size);
typedef void (*release_fn)(void* ctx);
typedef void (*reset_fn)(void* ctx);

typedef struct Allocator
{
	alloc_fn alloc;     // Allocate memory
	realloc_fn realloc; // Reallocate memory
	free_fn free;       // Free memory
	release_fn release; // Release all memory (destroy allocator)
	reset_fn reset;     // Reset allocator state

	void* ctx;
} Allocator;

typedef struct ArenaAllocator
{
	Allocator* base_allocator; // Allocator used by this arena
	void* memory;              // Pointer to the allocated memory
	u64 size;                  // Size of the allocated memory
	u64 used;                  // Amount of memory used
	u64 allocations;           // Number of allocations made
} ArenaAllocator;

Allocator allocator_get_heap_allocator(void);                                     // kinda global
Allocator allocator_get_arena_allocator(Allocator* base_allocator, u64 size); // creates new per call
