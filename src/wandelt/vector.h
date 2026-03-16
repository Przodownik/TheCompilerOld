/**
 * @file vector.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"
#include "wandelt/memory.h"

typedef struct VectorHeader
{
	Allocator* allocator;
	u64 length;   // Number of elements in the vector
	u64 capacity; // Capacity of the vector (number of elements it can hold)
	u64 stride;   // Size of each element in the vector (in bytes)
} VectorHeader;

#define VECTOR_RESIZE_FACTOR 2
#define VECTOR_HEADER(v)     ((VectorHeader*)(void*)((u8*)(v) - sizeof(VectorHeader)))

void* vector_create(Allocator* allocator, u64 initial_capacity, u64 stride);
void vector_destroy(void* vector);

#define vector_push(vector, value)                                            \
	{                                                                         \
		typeof(value) __temp_value__ = value;                                 \
		vector                       = _vector_push(vector, &__temp_value__); \
	}

#define vector_push_cstr(vector, str_literal)                           \
	{                                                                   \
		const char* __temp_str__ = str_literal;                         \
		vector                   = _vector_push(vector, &__temp_str__); \
	}

#define vector_insert_at(vector, index, value)                                         \
	{                                                                                  \
		typeof(value) __temp_value__ = value;                                          \
		vector                       = _vector_insert(vector, index, &__temp_value__); \
	}

u64 vector_get_capacity(void* vector);
u64 vector_get_length(void* vector);
u64 vector_get_stride(void* vector);

void* vector_at(void* vector, u64 index);
void* vector_front(void* vector);
void* vector_back(void* vector);
void vector_clear(void* vector);

bool vector_pop(void* vector, void* out_value);
bool vector_is_empty(void* vector);
bool vector_remove_at(void* vector, u64 index, void* out_value);

void* _vector_resize(void* vector);
void* _vector_push(void* vector, const void* value);
void* _vector_insert(void* vector, u64 index, const void* value);
void _vector_set_length(void* vector, u64 value);
