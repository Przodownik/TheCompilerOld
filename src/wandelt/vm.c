#include "vm.h"

VM vm_create(Chunk* chunk, CompiledFunction* functions, u8 function_count)
{
	VM vm;
	memset(&vm, 0, sizeof(VM));
	vm.chunk          = chunk;
	vm.ip             = chunk->instructions;
	vm.functions      = functions;
	vm.function_count = function_count;
	return vm;
}

VmResult vm_execute(VM* vm)
{
	static_assert(OP_CODE_COUNT == 52, "vm_execute needs to be updated for new opcodes");

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

		case OP_CODE_CALL: {
			u8 dest      = DECODE_A(inst);
			u8 first_arg = DECODE_B(inst);
			u8 func_idx  = DECODE_C(inst);

			ASSERT(func_idx < vm->function_count, "Invalid function index");
			CompiledFunction* fn = &vm->functions[func_idx];

			// Push call frame
			ASSERT(vm->frame_count < VM_MAX_CALL_DEPTH, "Call stack overflow");
			CallFrame* frame = &vm->call_stack[vm->frame_count++];
			frame->chunk     = vm->chunk;
			frame->return_ip = vm->ip;
			frame->reg_base  = vm->reg_base;
			frame->dest_reg  = vm->reg_base + dest;

			// New register window starts after caller's args
			u16 new_base = vm->reg_base + first_arg;
			ASSERT(new_base + 256 <= VM_MAX_REGISTERS || fn->param_count <= (VM_MAX_REGISTERS - new_base),
			       "Register file overflow");

			// Arguments are already in R[first_arg..first_arg+param_count) from caller's perspective,
			// which maps to R[0..param_count) in the callee's window since new_base = reg_base + first_arg

			// Switch to function
			vm->reg_base = new_base;
			vm->chunk    = &fn->chunk;
			vm->ip       = fn->chunk.instructions;
			R            = vm->registers + vm->reg_base;
			K            = vm->chunk->constants;
			break;
		}

		case OP_CODE_RETURN: {
			u8 a         = DECODE_A(inst);
			Value retval = R[a];

			if (vm->frame_count == 0)
			{
				vm->return_value = retval;
				return VM_OK;
			}

			// Pop call frame
			CallFrame* frame = &vm->call_stack[--vm->frame_count];
			vm->chunk    = frame->chunk;
			vm->ip       = frame->return_ip;
			vm->reg_base = frame->reg_base;

			// Write return value into caller's dest register
			vm->registers[frame->dest_reg] = retval;

			// Re-base R and K
			R = vm->registers + vm->reg_base;
			K = vm->chunk->constants;
			break;
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
