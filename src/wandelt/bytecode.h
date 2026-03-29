/**
 * @file bytecode.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast.h"
#include "wandelt/defines.h"
#include "wandelt/file.h"
#include "wandelt/memory.h"

// Each instruction is a 32-bit word. Three encoding formats:
//     [opcode:8][A:8][B:8][C:8]
//     [opcode:8][A:8][Bx:16]
//     [opcode:8][Ax:24]
// A = destination register
// B, C = source registers
// Bx = 16-bit unsigned immediate
// Ax = 24-bit unsigned immediate

typedef u32 Instruction;

typedef enum OpCode
{
	OP_CODE_LOAD_CONST, // 	ABx     R(A) = K(Bx)
	OP_CODE_MOVE,       // 	ABC     R(A) = R(B)

	OP_CODE_ADD_I, // 		ABC     R(A).i64 = R(B).i64 + R(C).i64
	OP_CODE_ADD_U, // 		ABC     R(A).u64 = R(B).u64 + R(C).u64
	OP_CODE_ADD_F, // 		ABC		R(A).f32 = R(B).f32 + R(C).f32
	OP_CODE_ADD_D, // 		ABC     R(A).f64 = R(B).f64 + R(C).f64

	OP_CODE_SUB_I, // 		ABC     R(A).i64 = R(B).i64 - R(C).i64
	OP_CODE_SUB_U, // 		ABC     R(A).u64 = R(B).u64 - R(C).u64
	OP_CODE_SUB_F, // 		ABC     R(A).f32 = R(B).f32 - R(C).f32
	OP_CODE_SUB_D, // 		ABC     R(A).f64 = R(B).f64 - R(C).f64

	OP_CODE_MUL_I, // 		ABC     R(A).i64 = R(B).i64 * R(C).i64
	OP_CODE_MUL_U, // 		ABC     R(A).u64 = R(B).u64 * R(C).u64
	OP_CODE_MUL_F, // 		ABC     R(A).f32 = R(B).f32 * R(C).f32
	OP_CODE_MUL_D, // 		ABC     R(A).f64 = R(B).f64 * R(C).f64

	OP_CODE_DIV_I, //  		ABC     R(A).i64 = R(B).i64 / R(C).i64
	OP_CODE_DIV_U, //  		ABC     R(A).u64 = R(B).u64 / R(C).u64
	OP_CODE_DIV_F, //  		ABC     R(A).f32 = R(B).f32 / R(C).f32
	OP_CODE_DIV_D, //  		ABC     R(A).f64 = R(B).f64 / R(C).f64

	OP_CODE_NEG_I, //		AB		R(A).i64 = -R(B).i64
	OP_CODE_NEG_F, //		AB		R(A).f32 = -R(B).f32
	OP_CODE_NEG_D, //		AB		R(A).f64 = -R(B).f64

	OP_CODE_EQ_I, //		ABC     R(A).bool = R(B).i64 == R(C).i64
	OP_CODE_EQ_U, //		ABC     R(A).bool = R(B).u64 == R(C).u64
	OP_CODE_EQ_F, //		ABC     R(A).bool = R(B).f32 == R(C).f32
	OP_CODE_EQ_D, //		ABC     R(A).bool = R(B).f64 == R(C).f64

	OP_CODE_NEQ_I, //		ABC     R(A).bool = R(B).i64 != R(C).i64
	OP_CODE_NEQ_U, //		ABC     R(A).bool = R(B).u64 != R(C).u64
	OP_CODE_NEQ_F, //		ABC     R(A).bool = R(B).f32 != R(C).f32
	OP_CODE_NEQ_D, //		ABC     R(A).bool = R(B).f64 != R(C).f64

	OP_CODE_LT_I, //		ABC     R(A).bool = R(B).i64 <  R(C).i64
	OP_CODE_LT_U, //		ABC     R(A).bool = R(B).u64 <  R(C).u64
	OP_CODE_LT_F, //		ABC     R(A).bool = R(B).f32 <  R(C).f32
	OP_CODE_LT_D, //		ABC     R(A).bool = R(B).f64 <  R(C).f64

	OP_CODE_GT_I, //		ABC     R(A).bool = R(B).i64 >  R(C).i64
	OP_CODE_GT_U, //		ABC     R(A).bool = R(B).u64 >  R(C).u64
	OP_CODE_GT_F, //		ABC     R(A).bool = R(B).f32 >  R(C).f32
	OP_CODE_GT_D, //		ABC     R(A).bool = R(B).f64 >  R(C).f64

	OP_CODE_LEQ_I, //		ABC     R(A).bool = R(B).i64 <= R(C).i64
	OP_CODE_LEQ_U, //		ABC     R(A).bool = R(B).u64 <= R(C).u64
	OP_CODE_LEQ_F, //		ABC     R(A).bool = R(B).f32 <= R(C).f32
	OP_CODE_LEQ_D, //		ABC     R(A).bool = R(B).f64 <= R(C).f64

	OP_CODE_GEQ_I, //		ABC     R(A).bool = R(B).i64 >= R(C).i64
	OP_CODE_GEQ_U, //		ABC     R(A).bool = R(B).u64 >= R(C).u64
	OP_CODE_GEQ_F, //		ABC     R(A).bool = R(B).f32 >= R(C).f32
	OP_CODE_GEQ_D, //		ABC     R(A).bool = R(B).f64 >= R(C).f64

	OP_CODE_CAST, //        ABC     R(A) = convert(R(B), TypeKind(C))

	OP_CODE_RETURN, // return R(A)
	OP_CODE_HALT,   // stop execution
	OP_CODE_COUNT,
} OpCode;

const char* op_code_to_cstr(OpCode op);

#define UNUSED_REG 0
#define UNUSED_IMM 0

#define ENCODE_ABC(op, a, b, c) \
	((Instruction)(op) | ((Instruction)(a) << 8) | ((Instruction)(b) << 16) | ((Instruction)(c) << 24))
#define ENCODE_ABx(op, a, bx) ((Instruction)(op) | ((Instruction)(a) << 8) | ((Instruction)(bx) << 16))
#define ENCODE_Ax(op, ax)     ((Instruction)(op) | ((Instruction)(ax) << 8))
#define DECODE_OP(inst)       ((inst) & 0xFF)
#define DECODE_A(inst)        (((inst) >> 8) & 0xFF)
#define DECODE_B(inst)        (((inst) >> 16) & 0xFF)
#define DECODE_C(inst)        (((inst) >> 24) & 0xFF)
#define DECODE_Bx(inst)       (((inst) >> 16) & 0xFFFF)
#define DECODE_Ax(inst)       (((inst) >> 8) & 0xFFFFFF)

typedef enum ValueKind
{
	VALUE_KIND_INVALID,
	VALUE_KIND_BOOL,
	VALUE_KIND_I8,
	VALUE_KIND_U8,
	VALUE_KIND_I16,
	VALUE_KIND_U16,
	VALUE_KIND_I32,
	VALUE_KIND_U32,
	VALUE_KIND_I64,
	VALUE_KIND_U64,
	VALUE_KIND_F32,
	VALUE_KIND_F64,
	VALUE_KIND_COUNT,
} ValueKind;

typedef struct Value
{
	ValueKind kind;

	union {
		i64 i64_val; // bool, char, short, int, long
		u64 u64_val; // uchar, ushort, uint, ulong
		f32 f32_val; // float
		f64 f64_val; // double
	};
} Value;

#define value_bool(v) ((Value){.kind = VALUE_KIND_BOOL, .i64_val = (i64)(v)})
#define value_i8(v)   ((Value){.kind = VALUE_KIND_I8, .i64_val = (i64)(v)})
#define value_u8(v)   ((Value){.kind = VALUE_KIND_U8, .u64_val = (u64)(v)})
#define value_i16(v)  ((Value){.kind = VALUE_KIND_I16, .i64_val = (i64)(v)})
#define value_u16(v)  ((Value){.kind = VALUE_KIND_U16, .u64_val = (u64)(v)})
#define value_i32(v)  ((Value){.kind = VALUE_KIND_I32, .i64_val = (i64)(v)})
#define value_u32(v)  ((Value){.kind = VALUE_KIND_U32, .u64_val = (u64)(v)})
#define value_i64(v)  ((Value){.kind = VALUE_KIND_I64, .i64_val = (v)})
#define value_u64(v)  ((Value){.kind = VALUE_KIND_U64, .u64_val = (v)})
#define value_f32(v)  ((Value){.kind = VALUE_KIND_F32, .f32_val = (v)})
#define value_f64(v)  ((Value){.kind = VALUE_KIND_F64, .f64_val = (v)})

ValueKind value_kind_from_type_kind(TypeKind tk);
const char* value_kind_to_cstr(ValueKind kind);
void value_print(Value v, FILE* out);
Value value_convert(Value src, TypeKind target);

typedef struct Chunk
{
	Instruction* instructions;
	Value* constants;
	u32* lines;       // parallel to instructions: source line number
	u32 current_line; // set by compiler before emitting
} Chunk;

Chunk chunk_create(Allocator* alloc);
u32 chunk_emit(Chunk* chunk, Instruction inst);
u32 chunk_add_constant(Chunk* chunk, Value val);

typedef struct Variable
{
	StringView name;
	u8 reg;
} Variable;

typedef struct BytecodeCompiler
{
	Allocator* alloc;
	const File* source;
	Chunk current_chunk;
	u8 next_free_reg_idx;
	Variable variables[128];
	u8 local_count;
	// u8 max_reg;  // high-water mark of registers used
	// u8 scope_depth;
} BytecodeCompiler;

BytecodeCompiler bytecode_compiler_create(Allocator* alloc, const File* source);
Chunk bytecode_compiler_compile(BytecodeCompiler* compiler, Statement** program_statements);

u8 bytecode_compiler_allocate_register(BytecodeCompiler* compiler);
void bytecode_compiler_set_line_from_span(BytecodeCompiler* c, Span span); // for disassembly
OpCode bytecode_compiler_select_negate_opcode(Type* type);
bool bytecode_compiler_cast_needs_instruction(TypeKind from, TypeKind to);
OpCode bytecode_compiler_select_binary_opcode(BinaryOperator bin_op, Type* type);

void bytecode_compiler_compile_statement(BytecodeCompiler* compiler, Statement* stmt);
void bytecode_compiler_compile_declaration_statement(BytecodeCompiler* compiler, Statement* stmt);
void bytecode_compiler_compile_expression_statement(BytecodeCompiler* compiler, Statement* stmt);
void bytecode_compiler_compile_return_statement(BytecodeCompiler* compiler, Statement* stmt);

u8 bytecode_compiler_compile_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_constant_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_unary_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_binary_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_group_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_identifier_expression(BytecodeCompiler* compiler, Expression* expr);
u8 bytecode_compiler_compile_cast_expression(BytecodeCompiler* compiler, Expression* expr);
