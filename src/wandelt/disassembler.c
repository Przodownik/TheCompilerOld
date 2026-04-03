#include "disassembler.h"

#include "bytecode.h"
#include "defines.h"
#include "wandelt/vector.h"
#include <assert.h>

#define DISASM_LINE_WIDTH 68

static void print_separator(FILE* out, char ch)
{
	for (int i = 0; i < DISASM_LINE_WIDTH; i++) fputc(ch, out);
	fputc('\n', out);
}

// Extract a source line (1-based) from a File into buf. Trims leading whitespace.
static void get_source_line(const File* source, u32 line, char* buf, u64 buf_size)
{
	buf[0] = '\0';
	if (!source || line == 0)
		return;

	const char* content = source->content.data;
	u64 len             = source->content.len;

	// Find start of the requested line
	u32 current_line = 1;
	u64 pos          = 0;
	while (pos < len && current_line < line)
	{
		if (content[pos] == '\n')
			current_line++;
		pos++;
	}

	// Skip leading whitespace
	while (pos < len && (content[pos] == ' ' || content[pos] == '\t')) pos++;

	// Copy until end of line
	u64 start = pos;
	while (pos < len && content[pos] != '\n' && content[pos] != '\r') pos++;

	u64 line_len = pos - start;
	if (line_len >= buf_size)
		line_len = buf_size - 1;
	memcpy(buf, content + start, line_len);
	buf[line_len] = '\0';
}

static void format_instruction(Chunk* chunk, u32 offset, char* operands, u64 op_size, char* comment, u64 cm_size)
{
	static_assert(OP_CODE_COUNT == 50, "format_instruction needs to be updated for new opcodes");

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
			Value v = chunk->constants[bx];
			switch (v.kind)
			{
			case VALUE_KIND_BOOL:
				snprintf(val_buf, sizeof(val_buf), "%s", v.i64_val ? "true" : "false");
				break;
			case VALUE_KIND_I8:
				snprintf(val_buf, sizeof(val_buf), "%d", (int)(i8)v.i64_val);
				break;
			case VALUE_KIND_U8:
				snprintf(val_buf, sizeof(val_buf), "%u", (unsigned)(u8)v.u64_val);
				break;
			case VALUE_KIND_I16:
				snprintf(val_buf, sizeof(val_buf), "%d", (int)(i16)v.i64_val);
				break;
			case VALUE_KIND_U16:
				snprintf(val_buf, sizeof(val_buf), "%u", (unsigned)(u16)v.u64_val);
				break;
			case VALUE_KIND_I32:
				snprintf(val_buf, sizeof(val_buf), "%d", (int)v.i64_val);
				break;
			case VALUE_KIND_U32:
				snprintf(val_buf, sizeof(val_buf), "%u", (unsigned)v.u64_val);
				break;
			case VALUE_KIND_I64:
				snprintf(val_buf, sizeof(val_buf), "%lld", v.i64_val);
				break;
			case VALUE_KIND_U64:
				snprintf(val_buf, sizeof(val_buf), "%llu", v.u64_val);
				break;
			case VALUE_KIND_F32:
				snprintf(val_buf, sizeof(val_buf), "%f", (double)v.f32_val);
				break;
			case VALUE_KIND_F64:
				snprintf(val_buf, sizeof(val_buf), "%f", v.f64_val);
				break;
			default:
				snprintf(val_buf, sizeof(val_buf), "???");
				break;
			}
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
		u32 target = offset + 1 + bx + 1; // +1 for IP advance, +1 for 1-based display
		snprintf(operands, op_size, "%u", bx);
		snprintf(comment, cm_size, "jump to %04u", target);
		break;
	}

	case OP_CODE_JUMP_IF_FALSE: {
		u8 a       = DECODE_A(inst);
		u32 bx     = DECODE_Bx(inst);
		u32 target = offset + 1 + bx + 1; // +1 for IP advance, +1 for 1-based display
		snprintf(operands, op_size, "R%u, %u", a, bx);
		snprintf(comment, cm_size, "if R%u == false jump to %04u", a, target);
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

static void disassemble_instruction_stream(Chunk* chunk, u32 offset, FILE* out)
{
	Instruction inst = chunk->instructions[offset];
	OpCode op        = (OpCode)DECODE_OP(inst);

	char operands[64];
	char comment[64];
	format_instruction(chunk, offset, operands, sizeof(operands), comment, sizeof(comment));

	if (comment[0])
		fprintf(out, "    %04u  %08X  %-14s%-16s; %s\n", offset + 1, inst, op_code_to_cstr(op), operands, comment);
	else
		fprintf(out, "    %04u  %08X  %-14s%s\n", offset + 1, inst, op_code_to_cstr(op), operands);
}

static void disassemble_chunk_stream(Chunk* chunk, const char* name, const File* source, FILE* out)
{
	u32 num_constants    = (u32)vector_get_length(chunk->constants);
	u32 num_instructions = (u32)vector_get_length(chunk->instructions);

	// Header
	fprintf(out, "=== %s ", name);
	int name_len = (int)strlen(name);
	for (int i = 0; i < DISASM_LINE_WIDTH - 5 - name_len; i++) fputc('=', out);
	fputc('\n', out);

	fprintf(out, "  Constants    : %u\n", num_constants);
	fprintf(out, "  Instructions : %u\n", num_instructions);

	// Constants
	print_separator(out, '-');
	fprintf(out, "  Constant Pool:\n");

	if (num_constants == 0)
	{
		fprintf(out, "    (empty)\n");
	}
	else
	{
		for (u32 i = 0; i < num_constants; i++)
		{
			Value v = chunk->constants[i];
			fprintf(out, "    K%-4u= ", i + 1);
			value_print(v, out);
			fprintf(out, " (%s)\n", value_kind_to_cstr(v.kind));
		}
	}

	// Instructions
	print_separator(out, '-');
	fprintf(out, "  Code:\n");

	u32 last_line = 0;
	for (u32 i = 0; i < num_instructions; i++)
	{
		// Show source line when it changes
		u32 line = (chunk->lines && i < (u32)vector_get_length(chunk->lines)) ? chunk->lines[i] : 0;
		if (source && line > 0 && line != last_line)
		{
			char src_line[256];
			get_source_line(source, line, src_line, sizeof(src_line));
			if (src_line[0])
			{
				if (last_line > 0)
					fprintf(out, "\n");
				fprintf(out, "    -- L%u: %s\n", line, src_line);
			}
			last_line = line;
		}

		disassemble_instruction_stream(chunk, i, out);
	}

	// Footer
	print_separator(out, '=');
	fputc('\n', out);
}

static void disassemble_chunk_readable(Chunk* chunk, const char* name, const File* source, FILE* out)
{
	u32 num_constants    = (u32)vector_get_length(chunk->constants);
	u32 num_instructions = (u32)vector_get_length(chunk->instructions);

	// Header
	fprintf(out, "=== %s ===\n\n", name);

	// Constants as typed directives
	if (num_constants > 0)
	{
		fprintf(out, "\n");
		for (u32 i = 0; i < num_constants; i++)
		{
			Value v = chunk->constants[i];
			fprintf(out, ".const %-6s K%u = ", value_kind_to_cstr(v.kind), i + 1);
			value_print(v, out);
			fprintf(out, "\n");
		}
	}

	// Instructions — same format as console output
	fprintf(out, "\n");

	u32 last_line = 0;
	for (u32 i = 0; i < num_instructions; i++)
	{
		// Source line mapping
		u32 line = (chunk->lines && i < (u32)vector_get_length(chunk->lines)) ? chunk->lines[i] : 0;
		if (source && line > 0 && line != last_line)
		{
			char src_line[256];
			get_source_line(source, line, src_line, sizeof(src_line));
			if (src_line[0])
			{
				if (last_line > 0)
					fprintf(out, "\n");
				fprintf(out, "; -- L%u: %s\n", line, src_line);
			}
			last_line = line;
		}

		Instruction inst = chunk->instructions[i];
		OpCode op        = (OpCode)DECODE_OP(inst);

		char operands[64];
		char comment[64];
		format_instruction(chunk, i, operands, sizeof(operands), comment, sizeof(comment));

		if (comment[0])
			fprintf(out, "%04u  %08X  %-14s%-16s; %s\n", i + 1, inst, op_code_to_cstr(op), operands, comment);
		else
			fprintf(out, "%04u  %08X  %-14s%s\n", i + 1, inst, op_code_to_cstr(op), operands);
	}
}

void disassemble_chunk(Chunk* chunk, const char* name, const File* source)
{
	disassemble_chunk_stream(chunk, name, source, stdout);
}

void disassemble_instruction(Chunk* chunk, u32 offset)
{
	disassemble_instruction_stream(chunk, offset, stdout);
}

bool disassemble_chunk_to_file(Chunk* chunk, const char* name, const File* source, const char* filepath)
{
	FILE* f     = nullptr;
	errno_t err = fopen_s(&f, filepath, "w");
	ASSERT(err == 0, "Failed to open file for writing: %s", filepath);
	if (!f)
		return false;
	disassemble_chunk_readable(chunk, name, source, f);
	fclose(f);
	return true;
}
