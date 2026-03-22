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
//     [opcode:8][A:8][B:8][C:8]     3-register  (arithmetic)
//     [opcode:8][A:8][Bx:16]        reg+imm16   (load, jump)
// A = destination register
// B, C = source registers
// Bx = 16-bit unsigned immediate (constant index or jump offset)

typedef u32 Instruction;

typedef enum OpCode
{
	OP_CODE_LOAD_CONST, // R(A) = K(Bx)
	OP_CODE_LOAD_INT,   // R(A) = (i16)Bx
	OP_CODE_MOVE,       // R(A) = R(B)
	OP_CODE_ADD,        // R(A) = R(B) + R(C)
	OP_CODE_SUB,        // R(A) = R(B) - R(C)
	OP_CODE_MUL,        // R(A) = R(B) * R(C)
	OP_CODE_DIV,        // R(A) = R(B) / R(C)
	OP_CODE_RETURN,     // return R(A)
	OP_CODE_HALT,       // stop execution
	OP_CODE_COUNT,
} OpCode;

const char* op_code_to_cstr(OpCode op);

#define ENCODE_ABC(op, a, b, c) \
	((Instruction)(op) | ((Instruction)(a) << 8) | ((Instruction)(b) << 16) | ((Instruction)(c) << 24))
#define ENCODE_ABx(op, a, bx) ((Instruction)(op) | ((Instruction)(a) << 8) | ((Instruction)(bx) << 16))
#define DECODE_OP(inst)       ((inst) & 0xFF)
#define DECODE_A(inst)        (((inst) >> 8) & 0xFF)
#define DECODE_B(inst)        (((inst) >> 16) & 0xFF)
#define DECODE_C(inst)        (((inst) >> 24) & 0xFF)
#define DECODE_Bx(inst)       (((inst) >> 16) & 0xFFFF)

typedef enum ValueKind
{
	VALUE_KIND_INTEGER,
} ValueKind;

typedef struct Value
{
	ValueKind kind;

	union {
		i64 integer;
	};
} Value;

static inline Value value_int(i64 v)
{
	return (Value){.kind = VALUE_KIND_INTEGER, .integer = v};
}

typedef struct Chunk
{
	Instruction* instructions;
	Value* constants;
	u32* lines;       // parallel to instructions: source line number
	u32 current_line; // set by compiler before emitting
	u32 registers_needed;
} Chunk;

Chunk chunk_create(Allocator* alloc);
u32 chunk_emit(Chunk* chunk, Instruction inst);
u32 chunk_add_constant(Chunk* chunk, Value val);

typedef struct LocalVariable
{
	StringView name;
	u8 reg;
	u8 scope_depth;
} LocalVariable;

typedef struct BytecodeCompiler
{
	Allocator* alloc;
	const File* source;
	Chunk chunk;
	u8 next_reg; // next free register (max 255)
	u8 max_reg;  // high-water mark of registers used
	LocalVariable variables[128];
	u8 local_count;
	u8 scope_depth;
} BytecodeCompiler;

BytecodeCompiler bytecode_compiler_create(Allocator* alloc, const File* source);
Chunk bytecode_compiler_compile(BytecodeCompiler* compiler, Statement** program_statements);
