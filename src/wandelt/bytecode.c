#include "bytecode.h"
#include "defines.h"
#include "wandelt/ast.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"

const char* op_code_to_cstr(OpCode op)
{
	static_assert(OP_CODE_COUNT == 48, "Update this function when adding new opcodes");

	switch (op)
	{
	case OP_CODE_LOAD_CONST:
		return "LOAD_CONST";
	case OP_CODE_MOVE:
		return "MOVE";

	case OP_CODE_ADD_I:
		return "ADD_I";
	case OP_CODE_ADD_U:
		return "ADD_U";
	case OP_CODE_ADD_F:
		return "ADD_F";
	case OP_CODE_ADD_D:
		return "ADD_D";

	case OP_CODE_SUB_I:
		return "SUB_I";
	case OP_CODE_SUB_U:
		return "SUB_U";
	case OP_CODE_SUB_F:
		return "SUB_F";
	case OP_CODE_SUB_D:
		return "SUB_D";

	case OP_CODE_MUL_I:
		return "MUL_I";
	case OP_CODE_MUL_U:
		return "MUL_U";
	case OP_CODE_MUL_F:
		return "MUL_F";
	case OP_CODE_MUL_D:
		return "MUL_D";

	case OP_CODE_DIV_I:
		return "DIV_I";
	case OP_CODE_DIV_U:
		return "DIV_U";
	case OP_CODE_DIV_F:
		return "DIV_F";
	case OP_CODE_DIV_D:
		return "DIV_D";

	case OP_CODE_NEG_I:
		return "NEG_I";
	case OP_CODE_NEG_F:
		return "NEG_F";
	case OP_CODE_NEG_D:
		return "NEG_D";

	case OP_CODE_EQ_I:
		return "EQ_I";
	case OP_CODE_EQ_U:
		return "EQ_U";
	case OP_CODE_EQ_F:
		return "EQ_F";
	case OP_CODE_EQ_D:
		return "EQ_D";

	case OP_CODE_NEQ_I:
		return "NEQ_I";
	case OP_CODE_NEQ_U:
		return "NEQ_U";
	case OP_CODE_NEQ_F:
		return "NEQ_F";
	case OP_CODE_NEQ_D:
		return "NEQ_D";

	case OP_CODE_LT_I:
		return "LT_I";
	case OP_CODE_LT_U:
		return "LT_U";
	case OP_CODE_LT_F:
		return "LT_F";
	case OP_CODE_LT_D:
		return "LT_D";

	case OP_CODE_GT_I:
		return "GT_I";
	case OP_CODE_GT_U:
		return "GT_U";
	case OP_CODE_GT_F:
		return "GT_F";
	case OP_CODE_GT_D:
		return "GT_D";

	case OP_CODE_LEQ_I:
		return "LEQ_I";
	case OP_CODE_LEQ_U:
		return "LEQ_U";
	case OP_CODE_LEQ_F:
		return "LEQ_F";
	case OP_CODE_LEQ_D:
		return "LEQ_D";

	case OP_CODE_GEQ_I:
		return "GEQ_I";
	case OP_CODE_GEQ_U:
		return "GEQ_U";
	case OP_CODE_GEQ_F:
		return "GEQ_F";
	case OP_CODE_GEQ_D:
		return "GEQ_D";

	case OP_CODE_CAST:
		return "CAST";

	case OP_CODE_RETURN:
		return "RETURN";
	case OP_CODE_HALT:
		return "HALT";

	default:
		break;
	}

	ASSERT(false, "Invalid OpCode");
}

ValueKind value_kind_from_type_kind(TypeKind tk)
{
	switch (tk)
	{
	case TYPE_KIND_BOOL:
		return VALUE_KIND_BOOL;
	case TYPE_KIND_CHAR:
		return VALUE_KIND_I8;
	case TYPE_KIND_UCHAR:
		return VALUE_KIND_U8;
	case TYPE_KIND_SHORT:
		return VALUE_KIND_I16;
	case TYPE_KIND_USHORT:
		return VALUE_KIND_U16;
	case TYPE_KIND_INT:
		return VALUE_KIND_I32;
	case TYPE_KIND_UINT:
		return VALUE_KIND_U32;
	case TYPE_KIND_LONG:
		return VALUE_KIND_I64;
	case TYPE_KIND_ULONG:
		return VALUE_KIND_U64;
	case TYPE_KIND_FLOAT:
		return VALUE_KIND_F32;
	case TYPE_KIND_DOUBLE:
		return VALUE_KIND_F64;
	default:
		ASSERT(false, "Invalid TypeKind");
		return VALUE_KIND_I64;
	}
}

const char* value_kind_to_cstr(ValueKind kind)
{
	switch (kind)
	{
	case VALUE_KIND_BOOL:
		return "bool";
	case VALUE_KIND_I8:
		return "char";
	case VALUE_KIND_U8:
		return "uchar";
	case VALUE_KIND_I16:
		return "short";
	case VALUE_KIND_U16:
		return "ushort";
	case VALUE_KIND_I32:
		return "int";
	case VALUE_KIND_U32:
		return "uint";
	case VALUE_KIND_I64:
		return "long";
	case VALUE_KIND_U64:
		return "ulong";
	case VALUE_KIND_F32:
		return "float";
	case VALUE_KIND_F64:
		return "double";
	default:
		ASSERT(false, "Invalid ValueKind");
		return "???";
	}
}

void value_print(Value v, FILE* out)
{
	switch (v.kind)
	{
	case VALUE_KIND_BOOL:
		fprintf(out, "%s", v.i64_val ? "true" : "false");
		break;
	case VALUE_KIND_I8:
		fprintf(out, "%d", (int)(i8)v.i64_val);
		break;
	case VALUE_KIND_U8:
		fprintf(out, "%u", (unsigned)(u8)v.u64_val);
		break;
	case VALUE_KIND_I16:
		fprintf(out, "%d", (int)(i16)v.i64_val);
		break;
	case VALUE_KIND_U16:
		fprintf(out, "%u", (unsigned)(u16)v.u64_val);
		break;
	case VALUE_KIND_I32:
		fprintf(out, "%d", (int)v.i64_val);
		break;
	case VALUE_KIND_U32:
		fprintf(out, "%u", (unsigned)v.u64_val);
		break;
	case VALUE_KIND_I64:
		fprintf(out, "%lld", v.i64_val);
		break;
	case VALUE_KIND_U64:
		fprintf(out, "%llu", v.u64_val);
		break;
	case VALUE_KIND_F32:
		fprintf(out, "%f", (double)v.f32_val);
		break;
	case VALUE_KIND_F64:
		fprintf(out, "%f", v.f64_val);
		break;
	default:
		fprintf(out, "???");
		break;
	}
}

static OpCode select_typed_opcode(BinaryOperator bin_op, Type* type)
{
	// Family: 0=I, 1=U, 2=F, 3=D
	int family;
	if (type->kind == TYPE_KIND_DOUBLE)
		family = 3;
	else if (type->kind == TYPE_KIND_FLOAT)
		family = 2;
	else if (type_is_unsigned(type))
		family = 1;
	else
		family = 0;

	// Bool equality uses the I (signed) family — bool is stored in i64_val
	if (type_is_bool(type))
		family = 0;

	// Opcodes are laid out in groups of 4: I, U, F, D
	OpCode bases[] = {
	    [BINARY_OPERATOR_ADD] = OP_CODE_ADD_I, [BINARY_OPERATOR_SUB] = OP_CODE_SUB_I,
	    [BINARY_OPERATOR_MUL] = OP_CODE_MUL_I, [BINARY_OPERATOR_DIV] = OP_CODE_DIV_I,
	    [BINARY_OPERATOR_EQ] = OP_CODE_EQ_I,   [BINARY_OPERATOR_NEQ] = OP_CODE_NEQ_I,
	    [BINARY_OPERATOR_LT] = OP_CODE_LT_I,   [BINARY_OPERATOR_GT] = OP_CODE_GT_I,
	    [BINARY_OPERATOR_LEQ] = OP_CODE_LEQ_I, [BINARY_OPERATOR_GEQ] = OP_CODE_GEQ_I,
	};
	ASSERT(bin_op < sizeof(bases) / sizeof(bases[0]), "Invalid BinaryOperator");

	return (OpCode)(bases[bin_op] + family);
}

Value value_convert(Value src, TypeKind target)
{
	i64 as_i64 = 0;
	u64 as_u64 = 0;
	f64 as_f64 = 0.0;

	switch (src.kind)
	{
	case VALUE_KIND_BOOL:
	case VALUE_KIND_I8:
	case VALUE_KIND_I16:
	case VALUE_KIND_I32:
	case VALUE_KIND_I64:
		as_i64 = src.i64_val;
		as_u64 = (u64)src.i64_val;
		as_f64 = (f64)src.i64_val;
		break;
	case VALUE_KIND_U8:
	case VALUE_KIND_U16:
	case VALUE_KIND_U32:
	case VALUE_KIND_U64:
		as_i64 = (i64)src.u64_val;
		as_u64 = src.u64_val;
		as_f64 = (f64)src.u64_val;
		break;
	case VALUE_KIND_F32:
		as_f64 = (f64)src.f32_val;
		as_i64 = (i64)src.f32_val;
		as_u64 = (u64)src.f32_val;
		break;
	case VALUE_KIND_F64:
		as_f64 = src.f64_val;
		as_i64 = (i64)src.f64_val;
		as_u64 = (u64)src.f64_val;
		break;
	default:
		ASSERT(false, "Invalid ValueKind in value_convert");
		break;
	}

	switch (target)
	{
	case TYPE_KIND_BOOL:
		return value_bool(as_i64 != 0);
	case TYPE_KIND_CHAR:
		return value_i8((i8)as_i64);
	case TYPE_KIND_UCHAR:
		return value_u8((u8)as_u64);
	case TYPE_KIND_SHORT:
		return value_i16((i16)as_i64);
	case TYPE_KIND_USHORT:
		return value_u16((u16)as_u64);
	case TYPE_KIND_INT:
		return value_i32((i32)as_i64);
	case TYPE_KIND_UINT:
		return value_u32((u32)as_u64);
	case TYPE_KIND_LONG:
		return value_i64(as_i64);
	case TYPE_KIND_ULONG:
		return value_u64(as_u64);
	case TYPE_KIND_FLOAT:
		return value_f32((f32)as_f64);
	case TYPE_KIND_DOUBLE:
		return value_f64(as_f64);
	default:
		ASSERT(false, "Invalid target TypeKind in value_convert");
		return value_i64(0);
	}
}

static bool cast_needs_instruction(TypeKind from, TypeKind to)
{
	if (from == to)
		return false;

	bool from_signed = (from == TYPE_KIND_BOOL || from == TYPE_KIND_CHAR || from == TYPE_KIND_SHORT ||
	                    from == TYPE_KIND_INT || from == TYPE_KIND_LONG);
	bool to_signed   = (to == TYPE_KIND_BOOL || to == TYPE_KIND_CHAR || to == TYPE_KIND_SHORT || to == TYPE_KIND_INT ||
                      to == TYPE_KIND_LONG);
	bool from_unsigned =
	    (from == TYPE_KIND_UCHAR || from == TYPE_KIND_USHORT || from == TYPE_KIND_UINT || from == TYPE_KIND_ULONG);
	bool to_unsigned =
	    (to == TYPE_KIND_UCHAR || to == TYPE_KIND_USHORT || to == TYPE_KIND_UINT || to == TYPE_KIND_ULONG);

	// Same-family widening: no instruction needed
	// Both signed integers -> all stored in i64_val
	if (from_signed && to_signed)
		return false;

	// Both unsigned integers -> all stored in u64_val
	if (from_unsigned && to_unsigned)
		return false;

	// Everything else crosses families and needs conversion
	return true;
}

static u8 bytecode_compiler_compile_node(BytecodeCompiler* c, Statement* statement);
static u8 bytecode_compiler_compile_expr(BytecodeCompiler* c, Expression* expression);

BytecodeCompiler bytecode_compiler_create(Allocator* alloc, const File* source)
{
	return (BytecodeCompiler){
	    .alloc    = alloc,
	    .source   = source,
	    .chunk    = chunk_create(alloc),
	    .next_reg = 0,
	    .max_reg  = 0,
	};
}

Chunk chunk_create(Allocator* alloc)
{
	return (Chunk){
	    .instructions     = (Instruction*)vector_create(alloc, 32, sizeof(Instruction)),
	    .constants        = (Value*)vector_create(alloc, 8, sizeof(Value)),
	    .lines            = (u32*)vector_create(alloc, 32, sizeof(u32)),
	    .current_line     = 0,
	    .registers_needed = 0,
	};
}
u32 chunk_emit(Chunk* chunk, Instruction inst)
{
	u32 offset = (u32)vector_get_length(chunk->instructions);
	vector_push(chunk->instructions, inst);
	vector_push(chunk->lines, chunk->current_line);
	return offset;
}
u32 chunk_add_constant(Chunk* chunk, Value val)
{
	u32 index = (u32)vector_get_length(chunk->constants);
	vector_push(chunk->constants, val);
	return index;
}

Chunk bytecode_compiler_compile(BytecodeCompiler* compiler, Statement** program_statements)
{
	for (u64 i = 0; i < vector_get_length(program_statements); i++)
	{
		Statement* stmt = program_statements[i];
		bytecode_compiler_compile_node(compiler, stmt);
	}

	chunk_emit(&compiler->chunk, ENCODE_ABx(OP_CODE_HALT, 0, 0));

	compiler->chunk.registers_needed = compiler->max_reg;

	return compiler->chunk;
}

static void set_line_from_span(BytecodeCompiler* c, Span span)
{
	if (c->source)
	{
		FileLocation loc      = file_resolve_location(c->source, span.begin);
		c->chunk.current_line = loc.row;
	}
}

static u8 bytecode_compiler_compile_node(BytecodeCompiler* c, Statement* statement)
{
	set_line_from_span(c, statement->span);

	switch (statement->type)
	{
	case STATEMENT_TYPE_DECLARATION: {
		if (statement->decl_stmt.declaration->type == DECLARATION_TYPE_VARIABLE)
		{
			VariableDeclaration* var_decl = &statement->decl_stmt.declaration->variable;
			if (var_decl->initializer)
			{
				u8 reg               = bytecode_compiler_compile_expr(c, var_decl->initializer);
				LocalVariable* local = &c->variables[c->local_count++];
				local->name          = var_decl->name;
				local->reg           = reg;
				local->scope_depth   = c->scope_depth;
				return reg;
			}
		}
		return 0;
	}

	case STATEMENT_TYPE_RETURN: {
		u8 reg = bytecode_compiler_compile_expr(c, statement->return_stmt.expression);
		chunk_emit(&c->chunk, ENCODE_ABx(OP_CODE_RETURN, reg, 0));
		return reg;
	}
	default:
		ASSERT(false, "Cannot compile node kind %d", statement->type);
		return 0;
	}
}

static bool is_local_register(BytecodeCompiler* c, u8 reg)
{
	for (u8 i = 0; i < c->local_count; i++)
	{
		if (c->variables[i].reg == reg)
			return true;
	}
	return false;
}

static OpCode select_negate_opcode(Type* type)
{
	if (type->kind == TYPE_KIND_DOUBLE)
		return OP_CODE_NEG_D;
	else if (type->kind == TYPE_KIND_FLOAT)
		return OP_CODE_NEG_F;
	else
		return OP_CODE_NEG_I;
}

static u8 bytecode_compiler_compile_expr(BytecodeCompiler* c, Expression* expression)
{
	static_assert(EXPRESSION_TYPE_COUNT == 7, "Update this function when adding new expression types");

	set_line_from_span(c, expression->span);

	switch (expression->type)
	{
	case EXPRESSION_TYPE_CONSTANT: {
		u8 dest = c->next_reg++;
		if (c->next_reg > c->max_reg)
			c->max_reg = c->next_reg;

		Value val;
		switch (expression->constant.kind)
		{
		case CONSTANT_KIND_BOOLEAN:
			val = value_bool(expression->constant.boolean_value);
			break;
		case CONSTANT_KIND_INTEGER:
			val = value_i64((i64)expression->constant.integer_value);
			break;
		case CONSTANT_KIND_FLOAT:
			val = value_f32(expression->constant.float_value);
			break;
		case CONSTANT_KIND_DOUBLE:
			val = value_f64(expression->constant.double_value);
			break;
		default:
			ASSERT(false, "Invalid constant kind");
			break;
		}

		// If resolved_type differs from the natural constant type perform an implicit conversion now
		TypeKind target    = expression->resolved_type->kind;
		ValueKind natural  = val.kind;
		ValueKind expected = value_kind_from_type_kind(target);
		if (natural != expected)
		{
			val = value_convert(val, target);
		}

		u32 k = chunk_add_constant(&c->chunk, val);
		chunk_emit(&c->chunk, ENCODE_ABx(OP_CODE_LOAD_CONST, dest, k));
		return dest;
	}

	case EXPRESSION_TYPE_UNARY: {
		u8 src  = bytecode_compiler_compile_expr(c, expression->unary.operand);
		u8 dest = src;
		if (is_local_register(c, src))
		{
			dest = c->next_reg++;
			if (c->next_reg > c->max_reg)
				c->max_reg = c->next_reg;
		}

		OpCode op = select_negate_opcode(expression->resolved_type);
		chunk_emit(&c->chunk, ENCODE_ABC(op, dest, src, 0));

		return dest;
	}

	case EXPRESSION_TYPE_BINARY: {
		u8 save_next = c->next_reg;
		u8 left      = bytecode_compiler_compile_expr(c, expression->binary.left);
		u8 right     = bytecode_compiler_compile_expr(c, expression->binary.right);

		// reuse a temporary register, never overwrite a local
		u8 dest;
		bool left_is_local  = is_local_register(c, left);
		bool right_is_local = is_local_register(c, right);

		if (!left_is_local)
			dest = left;
		else if (!right_is_local)
			dest = right;
		else
		{
			// Both operands are locals — allocate a fresh register
			dest = c->next_reg++;
			if (c->next_reg > c->max_reg)
				c->max_reg = c->next_reg;
		}

		Type* opcode_type = binary_operator_is_comparison(expression->binary.operator)
		                        ? expression->binary.left->resolved_type
		                        : expression->resolved_type;
		OpCode op         = select_typed_opcode(expression->binary.operator, opcode_type);
		chunk_emit(&c->chunk, ENCODE_ABC(op, dest, left, right));

		// Reclaim temporaries above dest, but never below the save point
		u8 new_next = (u8)(dest + 1);
		c->next_reg = (new_next > save_next) ? new_next : save_next;

		return dest;
	}

	case EXPRESSION_TYPE_GROUP:
		return bytecode_compiler_compile_expr(c, expression->group.inner);

	case EXPRESSION_TYPE_IDENTIFIER: {
		for (i64 i = c->local_count - 1; i >= 0; i--)
		{
			LocalVariable* local = &c->variables[i];
			if (string_view_equals(local->name, expression->identifier.name))
			{
				return local->reg;
			}
		}
		ASSERT(false, "Undefined variable");
	}

	case EXPRESSION_TYPE_CAST: {
		u8 src        = bytecode_compiler_compile_expr(c, expression->cast.expression);
		TypeKind from = expression->cast.expression->resolved_type->kind;
		TypeKind to   = expression->resolved_type->kind;

		if (cast_needs_instruction(from, to))
		{
			u8 dest = src;
			if (is_local_register(c, src))
			{
				dest = c->next_reg++;
				if (c->next_reg > c->max_reg)
					c->max_reg = c->next_reg;
			}
			chunk_emit(&c->chunk, ENCODE_ABC(OP_CODE_CAST, dest, src, (u8)to));
			return dest;
		}
		return src;
	}

	default:
		break;
	}

	ASSERT(false, "Cannot compile expression kind %d", expression->type);
}
