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
	u32 size_in_bits; // 8, 16, 32, 64
} Type;

Type* type_get_builtin_int(void);
