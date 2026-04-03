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
	static_assert(OP_CODE_COUNT == 51, "vm_execute needs to be updated for new opcodes");

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
		case OP_CODE_MOVE: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			R[a] = R[b];
			break;
		}

		case OP_CODE_ADD_I: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].i64_val = R[b].i64_val + R[c].i64_val;
			R[a].kind    = VALUE_KIND_I64;
			break;
		}
		case OP_CODE_ADD_U: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].u64_val = R[b].u64_val + R[c].u64_val;
			R[a].kind    = VALUE_KIND_U64;
			break;
		}
		case OP_CODE_ADD_F: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f32_val = R[b].f32_val + R[c].f32_val;
			R[a].kind    = VALUE_KIND_F32;
			break;
		}
		case OP_CODE_ADD_D: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f64_val = R[b].f64_val + R[c].f64_val;
			R[a].kind    = VALUE_KIND_F64;
			break;
		}

		case OP_CODE_SUB_I: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].i64_val = R[b].i64_val - R[c].i64_val;
			R[a].kind    = VALUE_KIND_I64;
			break;
		}
		case OP_CODE_SUB_U: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].u64_val = R[b].u64_val - R[c].u64_val;
			R[a].kind    = VALUE_KIND_U64;
			break;
		}
		case OP_CODE_SUB_F: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f32_val = R[b].f32_val - R[c].f32_val;
			R[a].kind    = VALUE_KIND_F32;
			break;
		}
		case OP_CODE_SUB_D: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f64_val = R[b].f64_val - R[c].f64_val;
			R[a].kind    = VALUE_KIND_F64;
			break;
		}

		case OP_CODE_MUL_I: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].i64_val = R[b].i64_val * R[c].i64_val;
			R[a].kind    = VALUE_KIND_I64;
			break;
		}
		case OP_CODE_MUL_U: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].u64_val = R[b].u64_val * R[c].u64_val;
			R[a].kind    = VALUE_KIND_U64;
			break;
		}
		case OP_CODE_MUL_F: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f32_val = R[b].f32_val * R[c].f32_val;
			R[a].kind    = VALUE_KIND_F32;
			break;
		}
		case OP_CODE_MUL_D: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f64_val = R[b].f64_val * R[c].f64_val;
			R[a].kind    = VALUE_KIND_F64;
			break;
		}

		case OP_CODE_DIV_I: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			ASSERT(R[c].i64_val != 0, "Division by zero");
			R[a].i64_val = R[b].i64_val / R[c].i64_val;
			R[a].kind    = VALUE_KIND_I64;
			break;
		}
		case OP_CODE_DIV_U: {
			u8 a = DECODE_A(inst);
			u8 b = DECODE_B(inst);
			u8 c = DECODE_C(inst);
			ASSERT(R[c].u64_val != 0, "Division by zero");
			R[a].u64_val = R[b].u64_val / R[c].u64_val;
			R[a].kind    = VALUE_KIND_U64;
			break;
		}
		case OP_CODE_DIV_F: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f32_val = R[b].f32_val / R[c].f32_val;
			R[a].kind    = VALUE_KIND_F32;
			break;
		}
		case OP_CODE_DIV_D: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			u8 c         = DECODE_C(inst);
			R[a].f64_val = R[b].f64_val / R[c].f64_val;
			R[a].kind    = VALUE_KIND_F64;
			break;
		}

		case OP_CODE_NEG_I: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			R[a].i64_val = -R[b].i64_val;
			R[a].kind    = VALUE_KIND_I64;
			break;
		}
		case OP_CODE_NEG_F: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			R[a].f32_val = -R[b].f32_val;
			R[a].kind    = VALUE_KIND_F32;
			break;
		}
		case OP_CODE_NEG_D: {
			u8 a         = DECODE_A(inst);
			u8 b         = DECODE_B(inst);
			R[a].f64_val = -R[b].f64_val;
			R[a].kind    = VALUE_KIND_F64;
			break;
		}

		case OP_CODE_EQ_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val == R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_EQ_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val == R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_EQ_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val == R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_EQ_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val == R[DECODE_C(inst)].f64_val);
			break;
		case OP_CODE_NEQ_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val != R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_NEQ_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val != R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_NEQ_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val != R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_NEQ_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val != R[DECODE_C(inst)].f64_val);
			break;
		case OP_CODE_LT_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val < R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_LT_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val < R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_LT_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val < R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_LT_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val < R[DECODE_C(inst)].f64_val);
			break;
		case OP_CODE_GT_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val > R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_GT_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val > R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_GT_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val > R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_GT_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val > R[DECODE_C(inst)].f64_val);
			break;
		case OP_CODE_LEQ_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val <= R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_LEQ_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val <= R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_LEQ_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val <= R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_LEQ_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val <= R[DECODE_C(inst)].f64_val);
			break;
		case OP_CODE_GEQ_I:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].i64_val >= R[DECODE_C(inst)].i64_val);
			break;
		case OP_CODE_GEQ_U:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].u64_val >= R[DECODE_C(inst)].u64_val);
			break;
		case OP_CODE_GEQ_F:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f32_val >= R[DECODE_C(inst)].f32_val);
			break;
		case OP_CODE_GEQ_D:
			R[DECODE_A(inst)] = value_bool(R[DECODE_B(inst)].f64_val >= R[DECODE_C(inst)].f64_val);
			break;

		case OP_CODE_CAST: {
			u8 a            = DECODE_A(inst);
			u8 b            = DECODE_B(inst);
			TypeKind target = (TypeKind)DECODE_C(inst);
			R[a]            = value_convert(R[b], target);
			break;
		}

		case OP_CODE_JUMP: {
			u16 offset = DECODE_Bx(inst);
			vm->ip += offset;
			break;
		}

		case OP_CODE_JUMP_BACK: {
			u16 offset = DECODE_Bx(inst);
			vm->ip -= offset;
			break;
		}

		case OP_CODE_JUMP_IF_FALSE: {
			u8 a       = DECODE_A(inst);
			u16 offset = DECODE_Bx(inst);
			if (!R[a].i64_val)
				vm->ip += offset;
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
