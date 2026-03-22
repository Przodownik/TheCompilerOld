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
	TYPE_KIND_BOOL,   // boolean type, 8-bit
	TYPE_KIND_CHAR,   // signed 8-bit
	TYPE_KIND_UCHAR,  // unsigned 8-bit
	TYPE_KIND_SHORT,  // signed 16-bit
	TYPE_KIND_USHORT, // unsigned 16-bit
	TYPE_KIND_INT,    // signed 32-bit
	TYPE_KIND_UINT,   // unsigned 32-bit
	TYPE_KIND_LONG,   // signed 64-bit
	TYPE_KIND_ULONG,  // unsigned 64-bit
	TYPE_KIND_FLOAT,  // 32-bit IEEE 754
	TYPE_KIND_DOUBLE, // 64-bit IEEE 754
	TYPE_KIND_COUNT,
} TypeKind;

const char* type_kind_to_cstr(TypeKind kind);

typedef struct Type
{
	TypeKind kind;
	u32 size_in_bits;      // 8, 16, 32, 64
	u32 alignment_in_bits; // 0, 8, 16, 32, 64
} Type;

Type* type_get_builtin(TypeKind kind);

bool type_is_arithmetic(const Type* t);
bool type_is_integer(const Type* t);
bool type_is_floating(const Type* t);
bool type_is_signed(const Type* t);
bool type_is_unsigned(const Type* t);
bool type_is_bool(const Type* type);

bool type_is_implicitly_convertible(Type* from, Type* to);
Type* type_common(Type* left, Type* right);
