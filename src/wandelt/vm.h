/**
 * @file vm.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/bytecode.h"

#define VM_MAX_REGISTERS 256

typedef enum VmResult
{
	VM_OK,
	VM_ERROR,
} VmResult;

typedef struct VM
{
	Chunk* chunk;
	Instruction* ip;                   // instruction pointer
	Value registers[VM_MAX_REGISTERS]; // register file
	Value return_value;                // result of execution
} VM;

VM vm_create(Chunk* chunk);
VmResult vm_execute(VM* vm);
