/**
 * @file type.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"

typedef enum TypeKind
{
	TYPE_KIND_INVALID = 0,
	TYPE_KIND_INT, // int
	TYPE_KIND_COUNT,
} TypeKind;

const char* type_kind_to_cstr(TypeKind kind);

typedef struct Type
{
	TypeKind kind;
	u32 size_in_bits;      // 8, 16, 32, 64
	u32 alignment_in_bits; // 0, 8, 16, 32, 64
} Type;

Type* type_get_builtin_int(void);

bool type_is_arithmetic(Type* t);
bool type_is_integer(Type* t);
bool type_is_floating(Type* t);
bool type_is_signed(Type* t);
bool type_is_implicitly_convertible(Type* from, Type* to);
