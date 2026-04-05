#include "bytecode.h"
#include "defines.h"
#include "wandelt/ast.h"
#include "wandelt/string.h"
#include "wandelt/vector.h"

const char* op_code_to_cstr(OpCode op)
{
	static_assert(OP_CODE_COUNT == 52, "Update this function when adding new opcodes");

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

	case OP_CODE_JUMP:
		return "JUMP";
	case OP_CODE_JUMP_BACK:
		return "JUMP_BACK";
	case OP_CODE_JUMP_IF_FALSE:
		return "JUMP_NE";

	case OP_CODE_CALL:
		return "CALL";

	case OP_CODE_RETURN:
		return "RETURN";
	case OP_CODE_HALT:
		return "HALT";

	default:
		break;
	}

	ASSERT(false, "Invalid OpCode");
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

u8 bytecode_compiler_find_variable_register(BytecodeCompiler* compiler, StringView name)
{
	for (i64 i = compiler->local_count - 1; i >= 0; i--)
	{
		Variable* local = &compiler->variables[i];
		if (string_view_equals(local->name, name))
			return local->reg;
	}

	ASSERT(false, "Undefined variable in bytecode compiler");
}

void bytecode_compiler_set_line_from_span(BytecodeCompiler* c, Span span)
{
	ASSERT(c->source, "Source file is required to set line from span");

	FileLocation loc              = file_resolve_location(c->source, span.begin);
	c->current_chunk.current_line = loc.row;
}

OpCode bytecode_compiler_select_negate_opcode(Type* type)
{
	static_assert(TYPE_KIND_COUNT == 13, "Update this function when adding new TypeKinds");

	if (type->kind == TYPE_KIND_DOUBLE)
		return OP_CODE_NEG_D;

	if (type->kind == TYPE_KIND_FLOAT)
		return OP_CODE_NEG_F;

	return OP_CODE_NEG_I;
}

bool bytecode_compiler_cast_needs_instruction(TypeKind from, TypeKind to)
{
	static_assert(TYPE_KIND_COUNT == 13, "Update this function when adding new TypeKinds");

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
	ASSERT((u64)bin_op < sizeof(bases) / sizeof(bases[0]), "Invalid BinaryOperator");

	return (OpCode)(bases[bin_op] + family);
}

u32 bytecode_compiler_emit_jump(BytecodeCompiler* compiler, OpCode op, u8 reg)
{
	u32 offset = chunk_emit(&compiler->current_chunk, ENCODE_ABx(op, reg, 0xFFFF)); // placeholder for patching

	return offset;
}

void bytecode_compiler_patch_jump(BytecodeCompiler* compiler, u32 jump_offset)
{
	u32 current_offset = (u32)vector_get_length(compiler->current_chunk.instructions);
	u32 jump_distance = current_offset - jump_offset - 1; // -1 because IP is already advanced past the jump instruction

	ASSERT(jump_distance <= 0xFFFF, "Jump distance exceeds maximum");

	Instruction* inst = &compiler->current_chunk.instructions[jump_offset];
	u8 reg            = DECODE_A(*inst);
	*inst             = ENCODE_ABx((OpCode)(*inst & 0xFF), reg, jump_distance);
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
	static_assert(STATEMENT_TYPE_COUNT == 9, "Update this function when adding new statement types");

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

	case STATEMENT_TYPE_BLOCK:
		bytecode_compiler_compile_block_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_IF:
		bytecode_compiler_compile_if_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_FOR:
		bytecode_compiler_compile_for_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_WHILE:
		bytecode_compiler_compile_while_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_ASSIGNMENT:
		bytecode_compiler_compile_assignment_statement(compiler, stmt);
		break;

	case STATEMENT_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid statement.");
		break;
	}
}

void bytecode_compiler_compile_declaration_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	bytecode_compiler_compile_declaration(compiler, stmt->decl_stmt.declaration);
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

void bytecode_compiler_compile_block_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	const u8 current_local_count = compiler->local_count;
	const u8 current_scope_depth = compiler->scope_depth;

	// todo add proper push/pop scope mechanic
	compiler->scope_depth++;

	for (u64 i = 0; i < vector_get_length(stmt->block_stmt.statements); i++)
		bytecode_compiler_compile_statement(compiler, stmt->block_stmt.statements[i]);

	compiler->scope_depth = current_scope_depth;
	compiler->local_count = current_local_count;
}

void bytecode_compiler_compile_if_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	const u8 cond_reg      = bytecode_compiler_compile_expression(compiler, stmt->if_stmt.condition);
	const u32 jump_to_else = bytecode_compiler_emit_jump(compiler, OP_CODE_JUMP_IF_FALSE, cond_reg);

	bytecode_compiler_compile_statement(compiler, stmt->if_stmt.then_block);

	if (stmt->if_stmt.else_block)
	{
		const u32 jump_past_else = bytecode_compiler_emit_jump(compiler, OP_CODE_JUMP, 0);
		bytecode_compiler_patch_jump(compiler, jump_to_else);

		bytecode_compiler_compile_statement(compiler, stmt->if_stmt.else_block);
		bytecode_compiler_patch_jump(compiler, jump_past_else);
	}
	else
	{
		bytecode_compiler_patch_jump(compiler, jump_to_else);
	}
}

void bytecode_compiler_compile_for_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	bytecode_compiler_compile_declaration(compiler, stmt->for_stmt.initializer);

	const u32 loop_start = (u32)vector_get_length(compiler->current_chunk.instructions);
	const u8 cond_reg    = bytecode_compiler_compile_expression(compiler, stmt->for_stmt.condition);

	const u32 exit_jump = bytecode_compiler_emit_jump(compiler, OP_CODE_JUMP_IF_FALSE, cond_reg);

	bytecode_compiler_compile_statement(compiler, stmt->for_stmt.body);
	bytecode_compiler_compile_expression(compiler, stmt->for_stmt.update);

	u32 current     = (u32)vector_get_length(compiler->current_chunk.instructions);
	u32 back_offset = current - loop_start + 1;
	ASSERT(back_offset <= 0xFFFF, "Loop too large");

	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_JUMP_BACK, 0, (u16)back_offset));

	bytecode_compiler_patch_jump(compiler, exit_jump);
}

void bytecode_compiler_compile_while_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	const u32 loop_start = (u32)vector_get_length(compiler->current_chunk.instructions);
	const u8 cond_reg    = bytecode_compiler_compile_expression(compiler, stmt->while_stmt.condition);

	const u32 exit_jump = bytecode_compiler_emit_jump(compiler, OP_CODE_JUMP_IF_FALSE, cond_reg);

	bytecode_compiler_compile_statement(compiler, stmt->while_stmt.body);

	const u32 current     = (u32)vector_get_length(compiler->current_chunk.instructions);
	const u32 back_offset = current - loop_start + 1; // +1 because this instruction itself takes a slot
	ASSERT(back_offset <= 0xFFFF, "Loop too large");

	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_JUMP_BACK, 0, (u16)back_offset));

	bytecode_compiler_patch_jump(compiler, exit_jump);
}

void bytecode_compiler_compile_assignment_statement(BytecodeCompiler* compiler, Statement* stmt)
{
	AssignmentStatement* assign = &stmt->assign_stmt;
	u8 target_reg               = bytecode_compiler_find_variable_register(compiler, assign->target_identifier);

	if (assign->operator == ASSIGNMENT_OPERATOR_PURE)
	{
		u8 value_reg = bytecode_compiler_compile_expression(compiler, assign->value);
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(OP_CODE_MOVE, target_reg, value_reg, 0));
	}
	else
	{
		u8 value_reg          = bytecode_compiler_compile_expression(compiler, assign->value);
		BinaryOperator bin_op = assignment_operator_to_binary_operator(assign->operator);
		OpCode op             = bytecode_compiler_select_binary_opcode(bin_op, assign->target_type);
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(op, target_reg, target_reg, value_reg));
	}
}

u8 bytecode_compiler_compile_declaration(BytecodeCompiler* compiler, Declaration* decl)
{
	static_assert(DECLARATION_TYPE_COUNT == 4, "Update this function when adding new declaration types");

	switch (decl->type)
	{
	case DECLARATION_TYPE_INVALID:
		ASSERT(false, "Invalid declaration.");
		break;

	case DECLARATION_TYPE_NAMESPACE:
		break; // no op

	case DECLARATION_TYPE_VARIABLE:
		return bytecode_compiler_compile_variable_declaration(compiler, decl);

	case DECLARATION_TYPE_FUNCTION:
		return bytecode_compiler_compile_function_declaration(compiler, decl);

	case DECLARATION_TYPE_COUNT:
	default:
		ASSERT(false, "Invalid declaration.");
		break;
	}

	return 0;
}

u8 bytecode_compiler_compile_variable_declaration(BytecodeCompiler* compiler, Declaration* decl)
{
	u8 reg = bytecode_compiler_compile_expression(compiler, decl->variable.initializer);

	Variable* local = &compiler->variables[compiler->local_count++];
	local->name     = decl->variable.name;
	local->reg      = reg;

	return reg;
}

u8 bytecode_compiler_compile_function_declaration(BytecodeCompiler* compiler, Declaration* decl)
{
	// Save compiler state
	Chunk saved_chunk = compiler->current_chunk;
	u8 saved_reg      = compiler->next_free_reg_idx;
	u8 saved_locals   = compiler->local_count;
	u8 saved_scope    = compiler->scope_depth;

	// Fresh state for function body
	compiler->current_chunk     = chunk_create(compiler->alloc);
	compiler->next_free_reg_idx = 0;
	compiler->local_count       = 0;
	compiler->scope_depth       = 0;

	// Parameters become locals in registers 0..N-1
	u64 param_count = vector_get_length(decl->fn.parameters);
	for (u64 i = 0; i < param_count; i++)
	{
		u8 reg          = bytecode_compiler_allocate_register(compiler);
		Variable* local = &compiler->variables[compiler->local_count++];
		local->name     = decl->fn.parameters[i].name;
		local->reg      = reg;
	}

	// Register function before compiling body so recursive calls can resolve
	ASSERT(compiler->function_count < 64, "Exceeded maximum function count");
	u8 fn_idx            = compiler->function_count++;
	CompiledFunction* fn = &compiler->functions[fn_idx];
	fn->name             = decl->fn.name;
	fn->param_count      = (u8)param_count;

	bytecode_compiler_compile_statement(compiler, decl->fn.body);

	fn->chunk = compiler->current_chunk;

	// Restore compiler state
	compiler->current_chunk     = saved_chunk;
	compiler->next_free_reg_idx = saved_reg;
	compiler->local_count       = saved_locals;
	compiler->scope_depth       = saved_scope;

	return 0;
}

u8 bytecode_compiler_compile_expression(BytecodeCompiler* compiler, Expression* expr)
{
	static_assert(EXPRESSION_TYPE_COUNT == 9, "Update this function when adding new expression types");

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

	case EXPRESSION_TYPE_INCDEC:
		return bytecode_compiler_compile_incdec_expression(compiler, expr);

	case EXPRESSION_TYPE_CALL:
		return bytecode_compiler_compile_call_expression(compiler, expr);

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
	return bytecode_compiler_find_variable_register(compiler, expr->identifier.name);
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

u8 bytecode_compiler_compile_incdec_expression(BytecodeCompiler* compiler, Expression* expr)
{
	ASSERT(expr->incdec.operand->type == EXPRESSION_TYPE_IDENTIFIER);

	u8 var_reg = bytecode_compiler_find_variable_register(compiler, expr->incdec.operand->identifier.name);

	Value one_val;
	Type* var_type = expr->resolved_type;
	if (var_type->kind == TYPE_KIND_FLOAT)
		one_val = value_f32(1.0f);
	else if (var_type->kind == TYPE_KIND_DOUBLE)
		one_val = value_f64(1.0);
	else if (type_is_unsigned(var_type))
		one_val = value_u64(1);
	else
		one_val = value_i64(1);

	u8 one_reg = bytecode_compiler_allocate_register(compiler);
	u32 k      = chunk_add_constant(&compiler->current_chunk, one_val);

	chunk_emit(&compiler->current_chunk, ENCODE_ABx(OP_CODE_LOAD_CONST, one_reg, k));

	BinaryOperator bin_op = expr->incdec.is_increment ? BINARY_OPERATOR_ADD : BINARY_OPERATOR_SUB;
	OpCode op             = bytecode_compiler_select_binary_opcode(bin_op, var_type);
	if (expr->incdec.is_postfix)
	{
		// save old value, modify, return old
		u8 old_reg = bytecode_compiler_allocate_register(compiler);
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(OP_CODE_MOVE, old_reg, var_reg, 0));
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(op, var_reg, var_reg, one_reg));
		return old_reg;
	}
	else
	{
		// modify, return new value
		chunk_emit(&compiler->current_chunk, ENCODE_ABC(op, var_reg, var_reg, one_reg));
		return var_reg;
	}
}

u8 bytecode_compiler_compile_call_expression(BytecodeCompiler* compiler, Expression* expr)
{
	CallExpression* call = &expr->call;

	// Find function index, todo make it better
	u8 func_idx = 0;
	bool found  = false;
	for (u8 i = 0; i < compiler->function_count; i++)
	{
		if (string_view_equals(compiler->functions[i].name, call->function_name))
		{
			func_idx = i;
			found    = true;
			break;
		}
	}
	ASSERT(found, "Undefined function in bytecode compiler");

	// Compile arguments into consecutive registers
	FunctionDeclaration* fn_decl = &call->declaration_ref->fn;
	u64 param_count              = vector_get_length(fn_decl->parameters);
	u8 first_arg                 = compiler->next_free_reg_idx;

	for (u64 i = 0; i < param_count; i++) bytecode_compiler_allocate_register(compiler);

	for (u64 i = 0; i < param_count; i++)
	{
		u8 result = bytecode_compiler_compile_expression(compiler, call->arguments[i].value);
		if (result != first_arg + i)
			chunk_emit(&compiler->current_chunk, ENCODE_ABC(OP_CODE_MOVE, first_arg + (u8)i, result, 0));
	}

	// Destination register for return value
	u8 dest = bytecode_compiler_allocate_register(compiler);

	chunk_emit(&compiler->current_chunk, ENCODE_ABC(OP_CODE_CALL, dest, first_arg, func_idx));

	return dest;
}
