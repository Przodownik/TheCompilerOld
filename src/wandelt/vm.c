#include "vm.h"

VM vm_create(Chunk* chunk)
{
	VM vm;
	memset(&vm, 0, sizeof(VM));
	vm.chunk = chunk;
	vm.ip    = chunk->instructions;
	return vm;
}

VmResult vm_execute(VM* vm)
{
	Value* R = vm->registers;
	Value* K = vm->chunk->constants;
	for (;;)
	{
		Instruction inst = *vm->ip++;
		switch (DECODE_OP(inst))
		{
		case OP_CODE_LOAD_CONST: {
			u8 a   = DECODE_A(inst);
			u16 bx = DECODE_Bx(inst);
			R[a]   = K[bx];
			break;
		}
		case OP_CODE_LOAD_INT: {
			u8 a   = DECODE_A(inst);
			i16 bx = (i16)DECODE_Bx(inst);
			R[a]   = value_int((i64)bx);
			break;
		}
		case OP_CODE_MOVE: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			R[a] = R[b];
			break;
		}
		case OP_CODE_ADD: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			R[a] = value_int(R[b].integer + R[c].integer);
			break;
		}
		case OP_CODE_SUB: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			R[a] = value_int(R[b].integer - R[c].integer);
			break;
		}
		case OP_CODE_MUL: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			R[a] = value_int(R[b].integer * R[c].integer);
			break;
		}
		case OP_CODE_DIV: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			ASSERT(R[c].integer != 0, "Division by zero");
			R[a] = value_int(R[b].integer / R[c].integer);
			break;
		}
		case OP_CODE_RETURN: {
			u8 a             = DECODE_A(inst);
			vm->return_value = R[a];
			return VM_OK;
		}
		case OP_CODE_HALT: {
			return VM_OK;
		}
		default:
			ASSERT(false, "Unknown opcode: %d", DECODE_OP(inst));
			return VM_ERROR;
		}
	}
}
