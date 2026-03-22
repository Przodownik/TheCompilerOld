/**
 * @file string.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"
#include "wandelt/memory.h"

typedef struct String
{
	Allocator* alloc;
	char* data;
	u64 len;
} String;

String string_from_cstr(Allocator* alloc, const char* str);
String string_from_buffer(Allocator* alloc, u64 len);

void string_free(String* s);

#define FMT_STR_ARG(s) (int)(s).len, (s).data

typedef struct StringView
{
	const char* data;
	u64 len;
} StringView;

StringView string_view_from_cstr(const char* str);
StringView string_view_from_cstr_part(const char* str, u64 len);

bool string_view_equals(StringView a, StringView b);
