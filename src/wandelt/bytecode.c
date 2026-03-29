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

ValueKind value_kind_forom_type_kind(TypeKind tk)
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
	case VALUE_KIND_INVALID:
		ASSERT(false, "Invalid value.");
		break;

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

Chunk chunk_create(Allocator* alloc)
{
	return (Chunk){
	    .instructions = (Instruction*)vector_create(alloc, 32, sizeof(Instruction)),
	    .constants    = (Value*)vector_create(alloc, 8, sizeof(Value)),
	    .lines        = (u32*)vector_create(alloc, 32, sizeof(u32)),
	    .current_line = 0,
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

BytecodeCompiler bytecode_compiler_create(Allocator* alloc, const File* source)
{
	return (BytecodeCompiler){
	    .alloc = alloc, .source = source, .current_chunk = chunk_create(alloc), .local_count = 0u};
}

u8 bytecode_compiler_allocate_register(BytecodeCompiler* compiler)
{
	ASSERT(compiler->next_free_reg_idx < 255, "Exceeded maximum register count");
	return compiler->next_free_reg_idx++;
}

void bytecode_compiler_set_line_from_span(BytecodeCompiler* c, Span span)
{
	ASSERT(c->source, "Source file is required to set line from span");

	FileLocation loc              = file_resolve_location(c->source, span.begin);
	c->current_chunk.current_line = loc.row;
}

OpCode bytecode_compiler_select_negate_opcode(Type* type)
{
	static_assert(TYPE_KIND_COUNT == 12, "Update this function when adding new TypeKinds");

	if (type->kind == TYPE_KIND_DOUBLE)
		return OP_CODE_NEG_D;

	if (type->kind == TYPE_KIND_FLOAT)
		return OP_CODE_NEG_F;

	return OP_CODE_NEG_I;
}

bool bytecode_compiler_cast_needs_instruction(TypeKind from, TypeKind to)
{
	static_assert(TYPE_KIND_COUNT == 12, "Update this function when adding new TypeKinds");

	if (from == to)
		return false;

	const bool from_signed = (from == TYPE_KIND_BOOL || from == TYPE_KIND_CHAR || from == TYPE_KIND_SHORT ||
	                          from == TYPE_KIND_INT || from == TYPE_KIND_LONG);

	const bool to_signed = (to == TYPE_KIND_BOOL || to == TYPE_KIND_CHAR || to == TYPE_KIND_SHORT ||
	                        to == TYPE_KIND_INT || to == TYPE_KIND_LONG);

	const bool from_unsigned =
	    (from == TYPE_KIND_UCHAR || from == TYPE_KIND_USHORT || from == TYPE_KIND_UINT || from == TYPE_KIND_ULONG);

	const bool to_unsigned =
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

OpCode bytecode_compiler_select_binary_opcode(BinaryOperator bin_op, Type* type)
{
	static_assert(BINARY_OPERATOR_COUNT == 11, "Update this function when adding new binary operators");

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

Chunk bytecode_compiler_compile(BytecodeCompiler* compiler, Statement** program_statements)
{
	for (u64 i = 0; i < vector_get_length(program_statements); i++)
	{
		Statement* stmt = program_statements[i];
		bytecode_compiler_compile_statement(compiler, stmt);
	}

	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_HALT, 1, 0));

	return compiler->current_chunk;
}

void bytecode_compiler_compile_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	static_assert(STATEMENT_TYPE_COUNT == 4, "Update this function when adding new statement types");

	bytecode_compiler_set_line_from_span(compiler, stmt->span);

	switch (stmt->type)
	{
	case STATEMENT_TYPE_INVALID:
		ASSERT(false, "Invalid statement.");
		break;

	case STATEMENT_TYPE_DECLARATION:
		bytecode_compiler_compile_declaration_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_EXPRESSION:
		bytecode_compiler_compile_expression_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_RETURN:
		bytecode_compiler_compile_return_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid statement.");
		break;
	}
}

void bytecode_compiler_compile_declaration_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	static_assert(DECLARATION_TYPE_COUNT == 3, "Update this function when adding new declaration types");

	Declaration* decl = stmt->decl_stmt.declaration;

	if (decl->type == DECLARATION_TYPE_VARIABLE)
	{
		VariableDeclaration* var_decl = &stmt->decl_stmt.declaration->variable;

		u8 reg = bytecode_compiler_compile_expression(compiler, var_decl->initializer);

		Variable* local = &compiler->variables[compiler->local_count++];
		local->name     = var_decl->name;
		local->reg      = reg;
	}
}

void bytecode_compiler_compile_expression_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	bytecode_compiler_compile_expression(compiler, stmt->expr_stmt.expression);
}

void bytecode_compiler_compile_return_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	const u8 reg = bytecode_compiler_compile_expression(compiler, stmt->return_stmt.expression);
	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_RETURN, reg, UNUSED_REG));
}

u8 bytecode_compiler_compile_expression(BytecodeCompiler* compiler, Expression* expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 7, "Update this function when adding new expression types");

	bytecode_compiler_set_line_from_span(compiler, expr->span);

	switch (expr->type)
	{
	case EXPRESSION_TYPE_INVALID:
		ASSERT(false, "Invalid statement.");
		break;

	case EXPRESSION_TYPE_CONSTANT:
		return bytecode_compiler_compile_constant_expression(compiler, expr);

	case EXPRESSION_TYPE_UNARY:
		return bytecode_compiler_compile_unary_expression(compiler, expr);

	case EXPRESSION_TYPE_BINARY:
		return bytecode_compiler_compile_binary_expression(compiler, expr);

	case EXPRESSION_TYPE_GROUP:
		return bytecode_compiler_compile_group_expression(compiler, expr);

	case EXPRESSION_TYPE_IDENTIFIER:
		return bytecode_compiler_compile_identifier_expression(compiler, expr);

	case EXPRESSION_TYPE_CAST:
		return bytecode_compiler_compile_cast_expression(compiler, expr);

	case EXPRESSION_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid expression.");
		break;
	}
}

u8 bytecode_compiler_compile_constant_expression(BytecodeCompiler* compiler, Expression* expr)
{
	u8 dest = bytecode_compiler_allocate_register(compiler);

	Value val;
	switch (expr->constant.kind)
	{
	case CONSTANT_KIND_BOOLEAN:
		val = value_bool(expr->constant.boolean_value);
		break;
	case CONSTANT_KIND_INTEGER:
		val = value_i64((i64)expr->constant.integer_value);
		break;
	case CONSTANT_KIND_FLOAT:
		val = value_f32(expr->constant.float_value);
		break;
	case CONSTANT_KIND_DOUBLE:
		val = value_f64(expr->constant.double_value);
		break;
	default:
		ASSERT(false, "Invalid constant kind");
		break;
	}

	u32 k = chunk_add_constant(&compiler->current_chunk, val);
	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_LOAD_CONST, dest, k));

	return dest;
}

u8 bytecode_compiler_compile_unary_expression(BytecodeCompiler* compiler, Expression* expr)
{
	u8 src  = bytecode_compiler_compile_expression(compiler, expr->unary.operand);
	u8 dest = bytecode_compiler_allocate_register(compiler);

	OpCode op = bytecode_compiler_select_negate_opcode(expr->resolved_type);
	chunk_emit(&compiler->current_chunk, ENCODE_ABC(op, dest, src, 0));

	return dest;
}

u8 bytecode_compiler_compile_binary_expression(BytecodeCompiler* compiler, Expression* expr)
{
	u8 left  = bytecode_compiler_compile_expression(compiler, expr->binary.left);
	u8 right = bytecode_compiler_compile_expression(compiler, expr->binary.right);
	u8 dest  = bytecode_compiler_allocate_register(compiler);

	Type* opcode_type =
	    binary_operator_is_comparison(expr->binary.operator) ? expr->binary.left->resolved_type : expr->resolved_type;

	OpCode op = bytecode_compiler_select_binary_opcode(expr->binary.operator, opcode_type);
	chunk_emit(&compiler->current_chunk, ENCODE_ABC(op, dest, left, right));

	return dest;
}

u8 bytecode_compiler_compile_group_expression(BytecodeCompiler* compiler, Expression* expr)
{
	return bytecode_compiler_compile_expression(compiler, expr->group.inner);
}

u8 bytecode_compiler_compile_identifier_expression(BytecodeCompiler* compiler, Expression* expr)
{
	for (i64 i = compiler->local_count - 1; i >= 0; i--)
	{
		Variable* local = &compiler->variables[i];

		if (string_view_equals(local->name, expr->identifier.name))
			return local->reg;
	}

	ASSERT(false, "Undefined variable");
}

u8 bytecode_compiler_compile_cast_expression(BytecodeCompiler* compiler, Expression* expr)
{
	u8 src        = bytecode_compiler_compile_expression(compiler, expr->cast.expression);
	TypeKind from = expr->cast.expression->resolved_type->kind;
	TypeKind to   = expr->resolved_type->kind;

	if (bytecode_compiler_cast_needs_instruction(from, to))
	{
		u8 dest = bytecode_compiler_allocate_register(compiler);
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(OP_CODE_CAST, dest, src, (u8)to));

		return dest;
	}

	return src;
}
