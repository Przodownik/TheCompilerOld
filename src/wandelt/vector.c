#include "vector.h"

void* vector_create(Allocator* allocator, u64 initial_capacity, u64 stride)
{
	ASSERT(initial_capacity > 0, "Initial capacity must be greater than 0");
	ASSERT(stride > 0, "Stride must be greater than 0");

	u64 header_size = sizeof(VectorHeader);

	u64 memory_block_size = initial_capacity * stride;
	ASSERT(memory_block_size / stride == initial_capacity, "Integer overflow in vector allocation");

	u64 total_size = header_size + memory_block_size;
	ASSERT(total_size >= header_size, "Integer overflow in total size calculation");

	void* memory = calloc(total_size, sizeof(i8));
	ASSERT(memory != nullptr, "Failed to allocate memory for vector");

	VectorHeader* header = memory;
	header->length       = 0u;
	header->capacity     = initial_capacity;
	header->stride       = stride;
	header->allocator    = allocator;

	return (u8*)memory + header_size;
}

void vector_destroy(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	u64 header_size = sizeof(VectorHeader);
	u64 total_size  = header_size + (header->capacity * header->stride);

	header->allocator->free(header->allocator->ctx, header, total_size);
}

u64 vector_get_capacity(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	return header->capacity;
}

u64 vector_get_length(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	return header->length;
}

u64 vector_get_stride(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	return header->stride;
}

void* vector_at(void* vector, u64 index)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	u64 address   = (u64)vector + (index * header->stride);
	void* element = (void*)address;

	ASSERT(element != nullptr, "Element at index is nullptr");

	return element;
}

void* vector_front(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	VectorHeader* header = VECTOR_HEADER(vector);
	ASSERT(header->length > 0, "Vector is empty");
	return vector_at(vector, 0);
}

void* vector_back(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	VectorHeader* header = VECTOR_HEADER(vector);
	ASSERT(header->length > 0, "Vector is empty");
	return vector_at(vector, header->length - 1);
}

void vector_clear(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	_vector_set_length(vector, 0);
}

bool vector_pop(void* vector, void* out_value)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	if (header->length == 0)
	{
		return false;
	}

	header->length--;

	if (out_value != nullptr)
	{
		u64 address = (u64)vector + (header->length * header->stride);
		memcpy(out_value, (void*)address, header->stride);
	}

	return true;
}

bool vector_is_empty(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	VectorHeader* header = VECTOR_HEADER(vector);
	return header->length == 0;
}

bool vector_remove_at(void* vector, u64 index, void* out_value)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	if (index >= header->length)
	{
		return false;
	}

	// Copy the value if requested
	if (out_value != nullptr)
	{
		u64 address = (u64)vector + (index * header->stride);
		memcpy(out_value, (void*)address, header->stride);
	}

	// Shift elements to the left
	if (index < header->length - 1)
	{
		u64 dest_address  = (u64)vector + (index * header->stride);
		u64 src_address   = dest_address + header->stride;
		u64 bytes_to_move = (header->length - index - 1) * header->stride;
		memmove((void*)dest_address, (void*)src_address, bytes_to_move);
	}

	header->length--;

	return true;
}

void* _vector_resize(void* vector)
{
	ASSERT(vector != nullptr, "Vector is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);
	u64 new_capacity     = header->capacity * VECTOR_RESIZE_FACTOR;

	ASSERT(new_capacity > header->capacity, "Integer overflow in vector resize");

	u64 header_size     = sizeof(VectorHeader);
	u64 old_memory_size = header->capacity * header->stride;
	u64 new_memory_size = new_capacity * header->stride;
	ASSERT(new_memory_size / header->stride == new_capacity, "Integer overflow in vector resize");

	u64 old_total_size = header_size + old_memory_size;
	u64 new_total_size = header_size + new_memory_size;
	ASSERT(new_total_size >= header_size, "Integer overflow in total size calculation");

	void* new_memory = header->allocator->realloc(header->allocator->ctx, header, old_total_size, new_total_size);
	ASSERT(new_memory != nullptr, "Failed to allocate memory for vector resize");

	VectorHeader* new_header = new_memory;
	new_header->capacity     = new_capacity;

	return (u8*)new_memory + header_size;
}

void* _vector_push(void* vector, const void* value)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	ASSERT(value != nullptr, "Value is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);

	if (header->length >= header->capacity)
	{
		vector = _vector_resize(vector);
		header = VECTOR_HEADER(vector);
	}

	u64 address = (u64)vector;
	address += (header->length * header->stride);

	memcpy((void*)address, value, header->stride);

	header->length++;

	return vector;
}

void* _vector_insert(void* vector, u64 index, const void* value)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	ASSERT(value != nullptr, "Value is nullptr");

	VectorHeader* header = VECTOR_HEADER(vector);
	ASSERT(index <= header->length, "Index out of bounds");

	// Resize if necessary
	if (header->length >= header->capacity)
	{
		vector = _vector_resize(vector);
		header = VECTOR_HEADER(vector);
	}

	// Shift elements to the right
	if (index < header->length)
	{
		u64 src_address   = (u64)vector + (index * header->stride);
		u64 dest_address  = src_address + header->stride;
		u64 bytes_to_move = (header->length - index) * header->stride;
		memmove((void*)dest_address, (void*)src_address, bytes_to_move);
	}

	// Insert the new element
	u64 address = (u64)vector + (index * header->stride);
	memcpy((void*)address, value, header->stride);

	header->length++;

	return vector;
}

void _vector_set_length(void* vector, u64 value)
{
	ASSERT(vector != nullptr, "Vector is nullptr");
	VectorHeader* header = VECTOR_HEADER(vector);
	ASSERT(value <= header->capacity, "Length exceeds capacity");
	header->length = value;
}
