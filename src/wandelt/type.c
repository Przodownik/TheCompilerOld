#include "type.h"

const char* type_kind_to_cstr(TypeKind kind)
{
	static_assert(TYPE_KIND_COUNT == 2, "update type_kind_to_cstr to handle new type kinds");

	switch (kind)
	{
	case TYPE_KIND_INVALID:
		return "invalid";
	case TYPE_KIND_INT:
		return "int";
	default:
		break;
	}

	ASSERT(false, "invalid type kind");
}

static Type int_type = {.kind = TYPE_KIND_INT, .size_in_bits = 32, .alignment_in_bits = 32};

Type* type_get_builtin_int(void)
{
	return &int_type;
}

bool type_is_arithmetic(Type* t)
{
	return t->kind == TYPE_KIND_INT;
}

bool type_is_integer(Type* t)
{
	return t->kind == TYPE_KIND_INT;
}

bool type_is_floating(Type* t)
{
	return false;
}

bool type_is_signed(Type* t)
{
	return false;
}

bool type_is_implicitly_convertible(Type* from, Type* to)
{
	return false;
}