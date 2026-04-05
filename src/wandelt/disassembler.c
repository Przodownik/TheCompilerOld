#include "disassembler.h"

#include "bytecode.h"
#include "defines.h"
#include "wandelt/vector.h"
#include <assert.h>

#define DISASM_LINE_WIDTH 68

// Extract a source line (1-based) from a File into buf. Trims leading whitespace.
static void get_source_line(const File* source, u32 line, char* buf, u64 buf_size)
{
	buf[0] = '\0';
	if (!source || line == 0)
		return;

	const char* content = source->content.data;
	u64 len             = source->content.len;

	u32 current_line = 1;
	u64 pos          = 0;
	while (pos < len && current_line < line)
	{
		if (content[pos] == '\n')
			current_line++;
		pos++;
	}

	while (pos < len && (content[pos] == ' ' || content[pos] == '\t')) pos++;

	u64 start = pos;
	while (pos < len && content[pos] != '\n' && content[pos] != '\r') pos++;

	u64 line_len = pos - start;
	if (line_len >= buf_size)
		line_len = buf_size - 1;
	memcpy(buf, content + start, line_len);
	buf[line_len] = '\0';
}

static void format_value(Value v, char* buf, u64 buf_size)
{
	switch (v.kind)
	{
	case VALUE_KIND_BOOL:
		snprintf(buf, buf_size, "%s", v.i64_val ? "true" : "false");
		break;
	case VALUE_KIND_I8:
		snprintf(buf, buf_size, "%d", (int)(i8)v.i64_val);
		break;
	case VALUE_KIND_U8:
		snprintf(buf, buf_size, "%u", (unsigned)(u8)v.u64_val);
		break;
	case VALUE_KIND_I16:
		snprintf(buf, buf_size, "%d", (int)(i16)v.i64_val);
		break;
	case VALUE_KIND_U16:
		snprintf(buf, buf_size, "%u", (unsigned)(u16)v.u64_val);
		break;
	case VALUE_KIND_I32:
		snprintf(buf, buf_size, "%d", (int)v.i64_val);
		break;
	case VALUE_KIND_U32:
		snprintf(buf, buf_size, "%u", (unsigned)v.u64_val);
		break;
	case VALUE_KIND_I64:
		snprintf(buf, buf_size, "%lld", v.i64_val);
		break;
	case VALUE_KIND_U64:
		snprintf(buf, buf_size, "%llu", v.u64_val);
		break;
	case VALUE_KIND_F32:
		snprintf(buf, buf_size, "%f", (double)v.f32_val);
		break;
	case VALUE_KIND_F64:
		snprintf(buf, buf_size, "%f", v.f64_val);
		break;
	default:
		snprintf(buf, buf_size, "???");
		break;
	}
}

static void format_instruction(Chunk* chunk, u32 offset, CompiledFunction* functions, u8 function_count,
                                char* operands, u64 op_size, char* comment, u64 cm_size)
{
	static_assert(OP_CODE_COUNT == 52, "format_instruction needs to be updated for new opcodes");

	Instruction inst = chunk->instructions[offset];
	OpCode op        = (OpCode)DECODE_OP(inst);

	operands[0] = '\0';
	comment[0]  = '\0';

	switch (op)
	{
	case OP_CODE_LOAD_CONST: {
		u8 a   = DECODE_A(inst);
		u16 bx = DECODE_Bx(inst);
		snprintf(operands, op_size, "R%u, K%u", a, bx);
		if (bx < (u16)vector_get_length(chunk->constants))
		{
			char val_buf[32];
			format_value(chunk->constants[bx], val_buf, sizeof(val_buf));
			snprintf(comment, cm_size, "R%u = %s", a, val_buf);
		}
		break;
	}
	case OP_CODE_MOVE: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		snprintf(operands, op_size, "R%u, R%u", a, b);
		snprintf(comment, cm_size, "R%u = R%u", a, b);
		break;
	}
	case OP_CODE_ADD_I:
	case OP_CODE_ADD_U:
	case OP_CODE_ADD_F:
	case OP_CODE_ADD_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, R%u", a, b, c);
		snprintf(comment, cm_size, "R%u = R%u + R%u", a, b, c);
		break;
	}
	case OP_CODE_SUB_I:
	case OP_CODE_SUB_U:
	case OP_CODE_SUB_F:
	case OP_CODE_SUB_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, R%u", a, b, c);
		snprintf(comment, cm_size, "R%u = R%u - R%u", a, b, c);
		break;
	}
	case OP_CODE_MUL_I:
	case OP_CODE_MUL_U:
	case OP_CODE_MUL_F:
	case OP_CODE_MUL_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, R%u", a, b, c);
		snprintf(comment, cm_size, "R%u = R%u * R%u", a, b, c);
		break;
	}
	case OP_CODE_DIV_I:
	case OP_CODE_DIV_U:
	case OP_CODE_DIV_F:
	case OP_CODE_DIV_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, R%u", a, b, c);
		snprintf(comment, cm_size, "R%u = R%u / R%u", a, b, c);
		break;
	}
	case OP_CODE_NEG_I:
	case OP_CODE_NEG_F:
	case OP_CODE_NEG_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		snprintf(operands, op_size, "R%u, R%u", a, b);
		snprintf(comment, cm_size, "R%u = -R%u", a, b);
		break;
	}

	case OP_CODE_EQ_I:
	case OP_CODE_EQ_U:
	case OP_CODE_EQ_F:
	case OP_CODE_EQ_D:
	case OP_CODE_NEQ_I:
	case OP_CODE_NEQ_U:
	case OP_CODE_NEQ_F:
	case OP_CODE_NEQ_D:
	case OP_CODE_LT_I:
	case OP_CODE_LT_U:
	case OP_CODE_LT_F:
	case OP_CODE_LT_D:
	case OP_CODE_GT_I:
	case OP_CODE_GT_U:
	case OP_CODE_GT_F:
	case OP_CODE_GT_D:
	case OP_CODE_LEQ_I:
	case OP_CODE_LEQ_U:
	case OP_CODE_LEQ_F:
	case OP_CODE_LEQ_D:
	case OP_CODE_GEQ_I:
	case OP_CODE_GEQ_U:
	case OP_CODE_GEQ_F:
	case OP_CODE_GEQ_D: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, R%u", a, b, c);
		const char* op_str = (op >= OP_CODE_EQ_I && op <= OP_CODE_EQ_D)     ? "=="
		                     : (op >= OP_CODE_NEQ_I && op <= OP_CODE_NEQ_D) ? "!="
		                     : (op >= OP_CODE_LT_I && op <= OP_CODE_LT_D)   ? "<"
		                     : (op >= OP_CODE_GT_I && op <= OP_CODE_GT_D)   ? ">"
		                     : (op >= OP_CODE_LEQ_I && op <= OP_CODE_LEQ_D) ? "<="
		                     : (op >= OP_CODE_GEQ_I && op <= OP_CODE_GEQ_D) ? ">="
		                                                                    : "??";
		snprintf(comment, cm_size, "R%u = R%u %s R%u", a, b, op_str, c);
		break;
	}

	case OP_CODE_CAST: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u", a, b);
		snprintf(comment, cm_size, "R%u = R%u as %s", a, b, type_kind_to_cstr((TypeKind)c));
		break;
	}

	case OP_CODE_JUMP: {
		u32 bx     = DECODE_Bx(inst);
		u32 target = offset + 1 + bx;
		snprintf(operands, op_size, "%u", bx);
		snprintf(comment, cm_size, "-> %04u", target);
		break;
	}

	case OP_CODE_JUMP_BACK: {
		u32 bx     = DECODE_Bx(inst);
		u32 target = offset + 1 - bx;
		snprintf(operands, op_size, "%u", bx);
		snprintf(comment, cm_size, "-> %04u", target);
		break;
	}

	case OP_CODE_JUMP_IF_FALSE: {
		u8 a       = DECODE_A(inst);
		u32 bx     = DECODE_Bx(inst);
		u32 target = offset + 1 + bx;
		snprintf(operands, op_size, "R%u, %u", a, bx);
		snprintf(comment, cm_size, "if !R%u -> %04u", a, target);
		break;
	}

	case OP_CODE_CALL: {
		u8 a = DECODE_A(inst);
		u8 b = DECODE_B(inst);
		u8 c = DECODE_C(inst);
		snprintf(operands, op_size, "R%u, R%u, %u", a, b, c);
		if (functions && c < function_count)
		{
			CompiledFunction* fn = &functions[c];
			snprintf(comment, cm_size, "R%u = %.*s(args from R%u)", a, (int)fn->name.len, fn->name.data, b);
		}
		else
		{
			snprintf(comment, cm_size, "R%u = call F%u(args from R%u)", a, c, b);
		}
		break;
	}
	case OP_CODE_RETURN: {
		u8 a = DECODE_A(inst);
		snprintf(operands, op_size, "R%u", a);
		snprintf(comment, cm_size, "return R%u", a);
		break;
	}
	case OP_CODE_HALT:
		snprintf(comment, cm_size, "stop");
		break;
	default:
		snprintf(operands, op_size, "???");
		break;
	}
}

static void emit_instruction_line(Chunk* chunk, u32 offset, u32 display_offset, CompiledFunction* functions,
                                  u8 function_count, FILE* out)
{
	Instruction inst = chunk->instructions[offset];
	OpCode op        = (OpCode)DECODE_OP(inst);

	char operands[64];
	char comment[64];
	format_instruction(chunk, offset, functions, function_count, operands, sizeof(operands), comment, sizeof(comment));

	if (comment[0])
		fprintf(out, "%04u  %-14s%-18s; %s\n", display_offset, op_code_to_cstr(op), operands, comment);
	else
		fprintf(out, "%04u  %-14s%s\n", display_offset, op_code_to_cstr(op), operands);
}

static void emit_source_comment(const File* source, u32 line, u32* last_line, FILE* out)
{
	if (!source || line == 0 || line == *last_line)
		return;

	char src_line[256];
	get_source_line(source, line, src_line, sizeof(src_line));
	if (src_line[0])
	{
		if (*last_line > 0)
			fprintf(out, "\n");
		fprintf(out, "; %s\n", src_line);
	}
	*last_line = line;
}

static void disassemble_program_stream(Chunk* main_chunk, CompiledFunction* functions, u8 function_count,
                                       const char* name, const File* source, FILE* out)
{
	(void)name;

	// Function table
	if (function_count > 0)
	{
		fprintf(out, "; === Function table ===\n");
		for (u8 i = 0; i < function_count; i++)
		{
			CompiledFunction* fn   = &functions[i];
			u32 num_instructions   = (u32)vector_get_length(fn->chunk.instructions);
			fprintf(out, "; F%u: \"%.*s\" (params=%u, instructions=%u)\n", i, (int)fn->name.len, fn->name.data,
			        fn->param_count, num_instructions);
		}
		fprintf(out, "\n");
	}

	// Constant pool — merge main + all function constants for display
	fprintf(out, "; === Constants ===\n");
	u32 main_num_constants = (u32)vector_get_length(main_chunk->constants);
	if (main_num_constants == 0 && function_count == 0)
	{
		fprintf(out, "; (empty)\n");
	}
	else
	{
		if (main_num_constants > 0)
		{
			fprintf(out, "; main:\n");
			for (u32 i = 0; i < main_num_constants; i++)
			{
				char val_buf[32];
				Value v = main_chunk->constants[i];
				format_value(v, val_buf, sizeof(val_buf));
				fprintf(out, ";   K%u = %s (%s)\n", i, val_buf, value_kind_to_cstr(v.kind));
			}
		}
		for (u8 fi = 0; fi < function_count; fi++)
		{
			u32 fn_consts = (u32)vector_get_length(functions[fi].chunk.constants);
			if (fn_consts > 0)
			{
				fprintf(out, "; %.*s:\n", (int)functions[fi].name.len, functions[fi].name.data);
				for (u32 i = 0; i < fn_consts; i++)
				{
					char val_buf[32];
					Value v = functions[fi].chunk.constants[i];
					format_value(v, val_buf, sizeof(val_buf));
					fprintf(out, ";   K%u = %s (%s)\n", i, val_buf, value_kind_to_cstr(v.kind));
				}
			}
		}
	}
	fprintf(out, "\n");

	// Code
	fprintf(out, "; === Code ===\n");

	u32 display_offset = 0;

	// Emit function bodies first
	for (u8 fi = 0; fi < function_count; fi++)
	{
		CompiledFunction* fn   = &functions[fi];
		u32 fn_instructions    = (u32)vector_get_length(fn->chunk.instructions);

		// Function header
		fprintf(out, "\n; --- function \"%.*s\" (", (int)fn->name.len, fn->name.data);
		for (u8 p = 0; p < fn->param_count; p++)
		{
			if (p > 0)
				fprintf(out, ", ");
			fprintf(out, "R%u", p);
		}
		fprintf(out, ") ---\n");

		u32 last_line = 0;
		for (u32 i = 0; i < fn_instructions; i++)
		{
			u32 line = (fn->chunk.lines && i < (u32)vector_get_length(fn->chunk.lines)) ? fn->chunk.lines[i] : 0;
			emit_source_comment(source, line, &last_line, out);
			emit_instruction_line(&fn->chunk, i, display_offset, functions, function_count, out);
			display_offset++;
		}
	}

	// Main code
	u32 main_instructions = (u32)vector_get_length(main_chunk->instructions);
	fprintf(out, "\n; --- main ---\n");

	u32 last_line = 0;
	for (u32 i = 0; i < main_instructions; i++)
	{
		u32 line = (main_chunk->lines && i < (u32)vector_get_length(main_chunk->lines)) ? main_chunk->lines[i] : 0;
		emit_source_comment(source, line, &last_line, out);
		emit_instruction_line(main_chunk, i, display_offset, functions, function_count, out);
		display_offset++;
	}

	fprintf(out, "\n");
}

void disassemble_program(Chunk* main_chunk, CompiledFunction* functions, u8 function_count, const char* name,
                         const File* source)
{
	disassemble_program_stream(main_chunk, functions, function_count, name, source, stdout);
}

bool disassemble_program_to_file(Chunk* main_chunk, CompiledFunction* functions, u8 function_count, const char* name,
                                 const File* source, const char* filepath)
{
	FILE* f     = nullptr;
	errno_t err = fopen_s(&f, filepath, "w");
	ASSERT(err == 0, "Failed to open file for writing: %s", filepath);
	if (!f)
		return false;
	disassemble_program_stream(main_chunk, functions, function_count, name, source, f);
	fclose(f);
	return true;
}
