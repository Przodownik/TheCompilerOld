/**
 * @file vm.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/bytecode.h"

#define VM_MAX_REGISTERS 8192
#define VM_MAX_CALL_DEPTH 256

typedef enum VmResult
{
	VM_OK,
	VM_ERROR,
} VmResult;

typedef struct CallFrame
{
	Chunk* chunk;
	Instruction* return_ip;
	u16 reg_base;
	u16 dest_reg; // absolute register index in caller's window for return value
} CallFrame;

typedef struct VM
{
	Chunk* chunk;
	Instruction* ip;
	Value registers[VM_MAX_REGISTERS];
	Value return_value;

	CompiledFunction* functions;
	u8 function_count;

	CallFrame call_stack[VM_MAX_CALL_DEPTH];
	u16 frame_count;
	u16 reg_base;
} VM;

VM vm_create(Chunk* chunk, CompiledFunction* functions, u8 function_count);
VmResult vm_execute(VM* vm);
