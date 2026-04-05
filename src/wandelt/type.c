#include "type.h"
#include "defines.h"

const char* type_kind_to_cstr(TypeKind kind)
{
	static_assert(TYPE_KIND_COUNT == 13, "update type_kind_to_cstr to handle new type kinds");

	switch (kind)
	{
	case TYPE_KIND_INVALID:
		ASSERT(false, "invalid type kind");
		break;

	case TYPE_KIND_BOOL:
		return "bool";

	case TYPE_KIND_CHAR:
		return "char";

	case TYPE_KIND_UCHAR:
		return "uchar";

	case TYPE_KIND_SHORT:
		return "short";

	case TYPE_KIND_USHORT:
		return "ushort";

	case TYPE_KIND_INT:
		return "int";

	case TYPE_KIND_UINT:
		return "uint";

	case TYPE_KIND_LONG:
		return "long";

	case TYPE_KIND_ULONG:
		return "ulong";

	case TYPE_KIND_FLOAT:
		return "float";

	case TYPE_KIND_DOUBLE:
		return "double";

	case TYPE_KIND_COUNT:
	default:
		ASSERT(false, "invalid type kind");
		break;
	}
}

static Type builtin_types[] = {
    [TYPE_KIND_VOID]   = {.kind = TYPE_KIND_VOID, .size_in_bits = 0, .alignment_in_bits = 0},
    [TYPE_KIND_BOOL]   = {.kind = TYPE_KIND_BOOL, .size_in_bits = 8, .alignment_in_bits = 8},
    [TYPE_KIND_CHAR]   = {.kind = TYPE_KIND_CHAR, .size_in_bits = 8, .alignment_in_bits = 8},
    [TYPE_KIND_UCHAR]  = {.kind = TYPE_KIND_UCHAR, .size_in_bits = 8, .alignment_in_bits = 8},
    [TYPE_KIND_SHORT]  = {.kind = TYPE_KIND_SHORT, .size_in_bits = 16, .alignment_in_bits = 16},
    [TYPE_KIND_USHORT] = {.kind = TYPE_KIND_USHORT, .size_in_bits = 16, .alignment_in_bits = 16},
    [TYPE_KIND_INT]    = {.kind = TYPE_KIND_INT, .size_in_bits = 32, .alignment_in_bits = 32},
    [TYPE_KIND_UINT]   = {.kind = TYPE_KIND_UINT, .size_in_bits = 32, .alignment_in_bits = 32},
    [TYPE_KIND_LONG]   = {.kind = TYPE_KIND_LONG, .size_in_bits = 64, .alignment_in_bits = 64},
    [TYPE_KIND_ULONG]  = {.kind = TYPE_KIND_ULONG, .size_in_bits = 64, .alignment_in_bits = 64},
    [TYPE_KIND_FLOAT]  = {.kind = TYPE_KIND_FLOAT, .size_in_bits = 32, .alignment_in_bits = 32},
    [TYPE_KIND_DOUBLE] = {.kind = TYPE_KIND_DOUBLE, .size_in_bits = 64, .alignment_in_bits = 64},
};

Type* type_get_builtin(TypeKind kind)
{
	ASSERT(kind > TYPE_KIND_INVALID && kind < TYPE_KIND_COUNT);
	return &builtin_types[kind];
}

bool type_is_arithmetic(const Type* type)
{
	return type_is_integer(type) || type_is_floating(type);
}

bool type_is_integer(const Type* type)
{
	return type->kind >= TYPE_KIND_CHAR && type->kind <= TYPE_KIND_ULONG;
}

bool type_is_floating(const Type* type)
{
	return type->kind == TYPE_KIND_FLOAT || type->kind == TYPE_KIND_DOUBLE;
}

bool type_is_signed(const Type* type)
{
	return type->kind == TYPE_KIND_CHAR || type->kind == TYPE_KIND_SHORT || type->kind == TYPE_KIND_INT ||
	       type->kind == TYPE_KIND_LONG;
}

bool type_is_unsigned(const Type* type)
{
	return type->kind == TYPE_KIND_BOOL || type->kind == TYPE_KIND_UCHAR || type->kind == TYPE_KIND_USHORT ||
	       type->kind == TYPE_KIND_UINT || type->kind == TYPE_KIND_ULONG;
}

bool type_is_bool(const Type* type)
{
	return type->kind == TYPE_KIND_BOOL;
}

bool type_is_void(const Type* type)
{
	return type->kind == TYPE_KIND_VOID;
}

bool type_is_implicitly_convertible(Type* from, Type* to)
{
	if (from == to)
		return true;

	// Bool is never implicitly convertible to/from anything
	if (from->kind == TYPE_KIND_BOOL || to->kind == TYPE_KIND_BOOL)
		return false;

	// Signed integer widening: char -> short -> int -> long
	if (type_is_signed(from) && type_is_signed(to))
	{
		return from->size_in_bits < to->size_in_bits;
	}

	// Unsigned integer widening: uchar -> ushort -> uint -> ulong
	if (type_is_unsigned(from) && type_is_unsigned(to))
	{
		return from->size_in_bits < to->size_in_bits;
	}

	// Unsigned-to-signed widening (e.g., uchar -> short, uint -> long)
	// Safe when signed type has strictly more bits than unsigned source
	if (type_is_unsigned(from) && type_is_signed(to))
	{
		return from->size_in_bits < to->size_in_bits;
	}

	// Float widening: float -> double
	if (type_is_floating(from) && type_is_floating(to))
	{
		return from->size_in_bits < to->size_in_bits;
	}

	// Integer-to-float: only when mantissa can represent all integer values
	// float has 24-bit mantissa  -> safe for <=16-bit integers
	// double has 53-bit mantissa -> safe for <=32-bit integers
	if (type_is_integer(from) && type_is_floating(to))
	{
		u32 mantissa_bits = (to->kind == TYPE_KIND_FLOAT) ? 24 : 53;
		return from->size_in_bits <= mantissa_bits / 2;
	}

	// Everything else requires explicit cast:
	// - Signed-to-unsigned (even same width)
	// - Narrowing of any kind
	// - Float-to-integer
	// - int/uint -> float (precision loss)
	// - long/ulong -> double (precision loss)
	return false;
}

bool type_is_explicitly_castable(const Type* from, const Type* to)
{
	ASSERT(from->kind != TYPE_KIND_INVALID && to->kind != TYPE_KIND_INVALID);

	if (from == to)
		return true;

	// Bool <-> any arithmetic: allowed
	if (type_is_bool(from) && type_is_arithmetic(to))
		return true;

	if (type_is_arithmetic(from) && type_is_bool(to))
		return true;

	// Any arithmetic <-> any arithmetic: allowed
	if (type_is_arithmetic(from) && type_is_arithmetic(to))
		return true;

	return false;
}

Type* type_common(Type* left, Type* right)
{
	if (left == right)
		return left;

	// If left can widen to right, result is right's type
	if (type_is_implicitly_convertible(left, right))
		return right;

	// If right can widen to left, result is left's type
	if (type_is_implicitly_convertible(right, left))
		return left;

	// No implicit conversion
	return nullptr;
}
