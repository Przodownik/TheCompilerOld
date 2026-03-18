#include "disassembler.h"

#include "bytecode.h"
#include "wandelt/vector.h"

void disassemble_chunk(Chunk* chunk, const char* name)
{
	printf("== %s ==\n", name);
	printf("Registers: %u\n", chunk->registers_needed);
	printf("Constants: %u\n", (u32)vector_get_length(chunk->constants));
	
	for (u32 i = 0; i < (u32)vector_get_length(chunk->constants); i++)
	{
		Value v = chunk->constants[i];
		printf("  K%u = %lld\n", i, v.integer);
	}
	
	printf("Code:\n");
	u32 count = (u32)vector_get_length(chunk->instructions);
	for (u32 i = 0; i < count; i++)
	{
		disassemble_instruction(chunk, i);
	}
	printf("== end %s ==\n\n", name);
}

void disassemble_instruction(Chunk* chunk, u32 offset)
{
	Instruction inst = chunk->instructions[offset];
	OpCode op        = (OpCode)DECODE_OP(inst);

	printf("  %04u  %-12s", offset, op_code_to_cstr(op));
	
	switch (op)
	{
	case OP_CODE_LOAD_CONST: {
		u8 a   = DECODE_A(inst);
		u16 bx = DECODE_Bx(inst);
		printf("R%u, K%u", a, bx);

		if (bx < (u16)vector_get_length(chunk->constants))
			printf("  ; %lld", chunk->constants[bx].integer);

		break;
	}
	case OP_CODE_LOAD_INT:
		printf("R%u, %d", DECODE_A(inst), (i16)DECODE_Bx(inst));
		break;
	case OP_CODE_MOVE:
		printf("R%u, R%u", DECODE_A(inst), DECODE_B(inst));
		break;
	case OP_CODE_ADD:
	case OP_CODE_SUB:
	case OP_CODE_MUL:
	case OP_CODE_DIV:
		printf("R%u, R%u, R%u", DECODE_A(inst), DECODE_B(inst), DECODE_C(inst));
		break;
	case OP_CODE_RETURN:
		printf("R%u", DECODE_A(inst));
		break;
	case OP_CODE_HALT:
		break;
	default:
		printf("???");
		break;
	}
	printf("\n");
}
